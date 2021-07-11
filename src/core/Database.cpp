// Copyright (c) 2020-2021 AVME Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include "Database.h"

// ======================================================================
// TOKEN DATABASE FUNCTIONS
// ======================================================================

bool Database::openTokenDB() {
  std::string path = Utils::walletFolderPath.string() + "/wallet/c-avax/tokens";
  if (!exists(path)) { create_directories(path); }
  this->tokenStatus = leveldb::DB::Open(this->tokenOpts, path, &this->tokenDB);
  return this->tokenStatus.ok();
}

std::string Database::getTokenDBStatus() {
  return this->tokenStatus.ToString();
}

void Database::closeTokenDB() {
  delete this->tokenDB;
}

std::string Database::getTokenDBValue(std::string key) {
  this->tokenStatus = this->tokenDB->Get(leveldb::ReadOptions(), key, &this->tokenValue);
  return (this->tokenStatus.ok()) ? this->tokenValue : this->tokenStatus.ToString();
}

bool Database::putTokenDBValue(std::string key, std::string value) {
  this->tokenStatus = this->tokenDB->Put(leveldb::WriteOptions(), key, value);
  return this->tokenStatus.ok();
}

bool Database::deleteTokenDBValue(std::string key) {
  this->tokenStatus = this->tokenDB->Delete(leveldb::WriteOptions(), key);
  return this->tokenStatus.ok();
}

std::vector<std::string> Database::getAllTokenDBValues() {
  std::vector<std::string> ret;
  leveldb::Iterator* it = this->tokenDB->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    ret.push_back(it->value().ToString());
  }
  delete it;
  return ret;
}

// ======================================================================
// TX HISTORY DATABASE FUNCTIONS
// ======================================================================

bool Database::openHistoryDB(std::string address) {
  std::string path = Utils::walletFolderPath.string()
    + "/wallet/c-avax/accounts/transactions/" + address;
  if (!exists(path)) { create_directories(path); }
  this->historyStatus = leveldb::DB::Open(this->historyOpts, path, &this->historyDB);
  return this->historyStatus.ok();
}

std::string Database::getHistoryDBStatus() {
  return this->historyStatus.ToString();
}

void Database::closeHistoryDB() {
  delete this->historyDB;
}

std::string Database::getHistoryDBValue(std::string key) {
  this->historyStatus = this->tokenDB->Get(leveldb::ReadOptions(), key, &this->historyValue);
  return (this->historyStatus.ok()) ? this->historyValue : this->historyStatus.ToString();
}

bool Database::putHistoryDBValue(std::string key, std::string value) {
  this->historyStatus = this->historyDB->Put(leveldb::WriteOptions(), key, value);
  return this->historyStatus.ok();
}

bool Database::deleteHistoryDBValue(std::string key) {
  this->historyStatus = this->historyDB->Delete(leveldb::WriteOptions(), key);
  return this->historyStatus.ok();
}

std::vector<std::string> Database::getAllHistoryDBValues() {
  std::vector<std::string> ret;
  leveldb::Iterator* it = this->historyDB->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    ret.push_back(it->value().ToString());
  }
  delete it;
  return ret;
}

