/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2020-2020 Bareos GmbH & Co. KG

   This program is Free Software; you can redistribute it and/or
   modify it under the terms of version three of the GNU Affero General Public
   License as published by the Free Software Foundation and included
   in the file LICENSE.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/
/**
 * @file
 * Output Formatter for Bareos Resources
 */

#ifndef BAREOS_LIB_OUTPUT_FORMATTER_RESOURCE_H_
#define BAREOS_LIB_OUTPUT_FORMATTER_RESOURCE_H_

#include "lib/alist.h"
#define NEED_JANSSON_NAMESPACE 1
#include "lib/output_formatter.h"

class PoolMem;

/**
 * Actual output formatter class.
 */
class OutputFormatterResource {
 private:
  /*
   * Members
   */
  OutputFormatter* send_;
  int indent_level_ = 0;

 private:
  /*
   * Methods
   */
  bool requiresEscaping(const char* o);
  void KeyMultipleStringsOnePerLineAddItem(const char* key,
                                           const char* item,
                                           bool as_comment,
                                           bool quoted_strings);


 public:
  /*
   * Methods
   */
  OutputFormatterResource(OutputFormatter* send, int indent_level = 0);
  ~OutputFormatterResource();

  OutputFormatter* GetOutputFormatter() { return send_; }

  std::string GetKeyFormatString(bool inherited,
                                 std::string baseformat = "%s = ");

  void ResourceTypeStart(const char* name, bool as_comment = false);
  void ResourceTypeEnd(const char* name, bool as_comment = false);

  void ResourceStart(const char* name);
  void ResourceEnd(const char* name);

  void SubResourceStart(const char* name,
                        bool as_comment = false,
                        std::string baseformat = "%s {\n");
  void SubResourceEnd(const char* name,
                      bool as_comment = false,
                      std::string baseformat = "}\n");

  void KeyBool(const char* name, bool value, bool as_comment = false);

  void KeySignedInt(const char* name, int64_t value, bool as_comment = false);
  void KeyUnsignedInt(const char* name, int64_t value, bool as_comment = false);

  void KeyString(const char* name, const char* value, bool as_comment = false)
  {
    KeyUnquotedString(name, value, as_comment);
  }
  void KeyString(const char* name, std::string value, bool as_comment = false)
  {
    KeyUnquotedString(name, value.c_str(), as_comment);
  }
  void KeyQuotedString(const char* name,
                       const char* value,
                       bool as_comment = false);
  void KeyQuotedString(const char* name,
                       std::string value,
                       bool as_comment = false);
  void KeyUnquotedString(const char* name,
                         const char* value,
                         bool as_comment = false);
  void KeyUnquotedString(const char* name,
                         std::string value,
                         bool as_comment = false);

  void KeyMultipleStringsInOneLine(const char* key,
                                   alist* list,
                                   bool as_comment = false,
                                   bool quoted_strings = true);

  void KeyMultipleStringsInOneLine(const char* key,
                                   alist* list,
                                   const char* (*GetValue)(void* item),
                                   bool as_comment = false,
                                   bool quoted_strings = true);

  void KeyMultipleStringsOnePerLine(const char* key,
                                    alist& list,
                                    bool as_comment = false,
                                    bool quoted_strings = true);

  void KeyMultipleStringsOnePerLine(const char* key,
                                    alist* list,
                                    bool as_comment = false,
                                    bool quoted_strings = true);

  void KeyMultipleStringsOnePerLine(const char* key,
                                    alist* list,
                                    const char* (*GetValue)(void* item),
                                    bool as_comment = false,
                                    bool quoted_strings = true);

  void KeyMultipleStringsOnePerLine(const char* key,
                                    const std::vector<std::string>&,
                                    bool as_comment = false,
                                    bool quoted_strings = true);

  void ArrayStart(const char* key,
                  bool as_comment = false,
                  std::string baseformat = "%s {\n");
  void ArrayEnd(const char* key,
                bool as_comment = false,
                std::string baseformat = "}\n");
};

#endif
