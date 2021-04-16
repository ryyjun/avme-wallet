// Copyright (c) 2020-2021 AVME Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#ifndef JSON_H
#define JSON_H

#include <iostream>
#include <string>

#include <json_spirit/JsonSpiritHeaders.h>

#ifdef __MINGW32__
#include <winsock2.h> // Windows.h asked for winsock2 to be included.
#include <windows.h>
#include <io.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

#include "Utils.h"

/**
 * Namespace for JSON manipulation (e.g. reading/writing/parsing/etc),
 * essentially a json_spirit wrapper.
 */
namespace JSON {
  /**
   * Mutex for avoiding two threads trying to simultaneously read/write,
   * even for different files. Better safe than sorry!
   */
  static std::mutex storageThreadLock;

  // Get object and array item from a JSON element, respectively.
  json_spirit::mValue objectItem(const json_spirit::mValue element, const std::string name);
  json_spirit::mValue arrayItem(const json_spirit::mValue element, size_t index);

  /**
   * Get a specific value from a JSON element.
   * Specify a delimiter to search nested values, e.g. "foo/bar" searches
   * for "bar" inside of "foo".
   * Returns the value in JSON format (which should call json_spirit's
   * get_str() or similar to properly get the data), or nothing on failure.
   */
  json_spirit::mValue getValue(std::string jsonStr, std::string value, std::string delim = "");

  /**
   * Get a string value from a JSON element.
   * Specify a delimiter to search nested values, e.g. "foo/bar" searches
   * for "bar" inside of "foo".
   * Returns the value as a string, or an empty string on failure.
   */
  std::string getString(std::string jsonStr, std::string value, std::string delim = "");

  // Handle the transaction history storage directory.
  #ifdef __MINGW32__
  boost::filesystem::path GetSpecialFolderPath(int nFolder, bool fCreate = true);
  #endif
  boost::filesystem::path getDefaultDataDir();
  boost::filesystem::path getDataDir();

  // Read/write JSON files.
  json_spirit::mValue readFile(boost::filesystem::path filePath);
  json_spirit::mValue writeFile(json_spirit::mObject jsonObject, boost::filesystem::path filePath);
};

#endif // JSON_H
