from bareos_libcloud_api.bucket_explorer import JOB_TYPE
from bareos_libcloud_api.process_base import ProcessBase
from bareos_libcloud_api.get_libcloud_driver import get_driver
import io
from libcloud.common.types import LibcloudError
from libcloud.storage.types import ObjectDoesNotExistError
import uuid
from time import sleep

FINISH = 0
CONTINUE = 1

class Worker(ProcessBase):
    def __init__(
        self,
        options,
        worker_id,
        tmp_dir_path,
        message_queue,
        discovered_objects_queue,
        downloaded_objects_queue,
    ):
        super(Worker, self).__init__(
            worker_id, message_queue,
        )
        self.options = options
        self.tmp_dir_path = tmp_dir_path
        self.input_queue = discovered_objects_queue
        self.output_queue = downloaded_objects_queue

    def run_process(self):
        self.driver = get_driver(self.options)
        if self.driver == None:
            self.error_message("Could not load driver")
            return

        status = CONTINUE
        while status != FINISH:
            status = self.__iterate_input_queue()

    def __iterate_input_queue(self):
        while not self.input_queue.empty():
            if self.shutdown_event.is_set():
                return FINISH
            job = self.input_queue.get()
            if job == None:  # poison pill
                return FINISH
            if self.__run_job(job) == FINISH:
                return FINISH
        # try again
        return CONTINUE

    def __run_job(self, job):
        try:
            obj = self.driver.get_object(job["bucket"], job["name"])
            if obj is None:
                # Object cannot be fetched, an error is already logged
                return CONTINUE
        except Exception as exception:
            self.worker_exception("Could not get file object", exception)
            return FINISH

        if job["size"] < 1024 * 10:
            try:
                stream = obj.as_stream()
                content = b"".join(list(stream))

                size_of_fetched_object = len(content)
                if size_of_fetched_object != job["size"]:
                    self.error_message(
                        "prefetched file %s: got %s bytes, not the real size (%s bytes)"
                        % (job["name"], size_of_fetched_object, job["size"]),
                    )
                    return FINISH

                job["data"] = io.BytesIO(content)
                job["type"] = JOB_TYPE.DOWNLOADED
            except LibcloudError as e:
                self.worker_exception("Libcloud error, could not download file", e)
                return FINISH
            except Exception as e:
                self.worker_exception("Could not download file", e)
                return FINISH
        elif job["size"] < self.options["prefetch_size"]:
            try:
                tmpfilename = self.tmp_dir_path + "/" + str(uuid.uuid4())
                obj.download(tmpfilename)
                job["data"] = None
                job["tmpfile"] = tmpfilename
                job["type"] = JOB_TYPE.TEMP_FILE
            except OSError as e:
                self.worker_exception("Could not open temporary file", e)
                return FINISH
            except ObjectDoesNotExistError as e:
                self.worker_exception("Could not open object", e)
                return CONTINUE
            except LibcloudError as e:
                self.worker_exception("Error downloading object", e)
                return FINISH
            except Exception as e:
                self.worker_exception("Error using temporary file", e)
                return FINISH
        else:
            try:
                job["data"] = obj
                job["type"] = JOB_TYPE.STREAM
            except LibcloudError as e:
                self.worker_exception("Libcloud error preparing stream object", e)
                return FINISH
            except Exception as e:
                self.worker_exception("Error preparing stream object", e)
                return FINISH

        self.queue_try_put(self.output_queue, job)

        # success
        return CONTINUE