// Aleth: Ethereum C++ client, tools and libraries.
// Copyright 2015-2019 Aleth Authors.
// Licensed under the GNU General Public License, Version 3.


#pragma once

#include <functional>
#include <mutex>
#include <lib/devcore/FileSystem.h>
#include <lib/devcore/CommonData.h>
#include <lib/devcrypto/SecretStore.h>

#include <boost/filesystem.hpp>

namespace dev
{
namespace eth
{
class PasswordUnknown: public Exception {};

struct KeyInfo
{
	KeyInfo() = default;
	KeyInfo(h256 const& _passHash, std::string const& _accountName, std::string const& _passwordHint = std::string()): passHash(_passHash), accountName(_accountName), passwordHint(_passwordHint) {}

	/// Hash of the password or h256() / UnknownPassword if unknown.
	h256 passHash;
	/// Name of the key, or JSON key info if begins with '{'.
	std::string accountName;
	/// Hint of the password. Alternative place for storage than the hash-based lookup.
	std::string passwordHint;
};

static h256 const UnknownPassword;
/// Password query function that never returns a password.
static auto const DontKnowThrow = [](){ throw PasswordUnknown(); return std::string(); };

enum class SemanticPassword
{
	Existing,
	Master
};

// TODO: This one is specifically for Ethereum, but we can make it generic in due course.
// TODO: hidden-partition style key-store.
/**
 * @brief High-level manager of password-encrypted keys for Ethereum.
 * Usage:
 *
 * Call exists() to check whether there is already a database. If so, get the master password from
 * the user and call load() with it. If not, get a new master password from the user (get them to type
 * it twice and keep some hint around!) and call create() with it.
 *
 * Uses a "key file" (and a corresponding .salt file) that contains encrypted information about the keys and
 * a directory called "secrets path" that contains a file for each key.
 */
class KeyManager
{
public:
	enum class NewKeyType { DirectICAP = 0, NoVanity, FirstTwo, FirstTwoNextTwo, FirstThree, FirstFour };

	KeyManager(boost::filesystem::path const& _keysFile = defaultPath(), boost::filesystem::path const& _secretsPath = SecretStore::defaultPath());
	~KeyManager();

	void setKeysFile(boost::filesystem::path const& _keysFile) { m_keysFile = _keysFile; }
	boost::filesystem::path const& keysFile() const { return m_keysFile; }

	bool exists() const;
	void create(std::string const& _pass);
	bool load(std::string const& _pass);
	void save(std::string const& _pass) const { write(_pass, m_keysFile); }

	void notePassword(std::string const& _pass) { m_cachedPasswords[hashPassword(_pass)] = _pass; }
	void noteHint(std::string const& _pass, std::string const& _hint) { if (!_hint.empty()) m_passwordHint[hashPassword(_pass)] = _hint; }
	bool haveHint(std::string const& _pass) const { auto h = hashPassword(_pass); return m_cachedPasswords.count(h) && !m_cachedPasswords.at(h).empty(); }

	/// @returns the list of account addresses.
	Addresses accounts() const;
	/// @returns a hashset of all account addresses.
	AddressHash accountsHash() const { return AddressHash() + accounts(); }
	bool hasAccount(Address const& _address) const;
	/// @returns the human-readable name or json-encoded info of the account for the given address.
	std::string const& accountName(Address const& _address) const;
	/// @returns the password hint for the account for the given address;
	std::string const& passwordHint(Address const& _address) const;

	/// @returns the uuid of the key for the address @a _a or the empty hash on error.
	h128 uuid(Address const& _a) const;
	/// @returns the address corresponding to the key with uuid @a _uuid or the zero address on error.
	Address address(h128 const& _uuid) const;

	h128 import(Secret const& _s, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	h128 import(Secret const& _s, std::string const& _accountName) { return import(_s, _accountName, defaultPassword(), std::string()); }

	SecretStore& store() { return m_store; }
	void importExisting(h128 const& _uuid, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	void importExisting(h128 const& _uuid, std::string const& _accountName, Address const& _addr, h256 const& _passHash = h256(), std::string const& _passwordHint = std::string());

	/// @returns the secret key associated with an address provided the password query
	/// function @a _pass or the zero-secret key on error.
	Secret secret(Address const& _address, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;
	/// @returns the secret key associated with the uuid of a key provided the password query
	/// function @a _pass or the zero-secret key on error.
	Secret secret(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;

	bool recode(Address const& _address, std::string const& _newPass, std::string const& _hint, std::function<std::string()> const& _pass = DontKnowThrow, KDF _kdf = KDF::Scrypt);

	void kill(h128 const& _id) { kill(address(_id)); }
	void kill(Address const& _a);

	static boost::filesystem::path defaultPath() { return getDataDir("ethereum") / boost::filesystem::path("keys.info"); }

	/// Extracts the secret key from the presale wallet.
	static KeyPair presaleSecret(std::string const& _json, std::function<std::string(bool)> const& _password);

	/// @returns new random keypair with given vanity
	static  KeyPair newKeyPair(NewKeyType _type);
private:
	std::string getPassword(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string getPassword(h256 const& _passHash, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string defaultPassword(std::function<std::string()> const& _pass = DontKnowThrow) const { return getPassword(m_master, _pass); }
	h256 hashPassword(std::string const& _pass) const;

	/// Stores the password by its hash in the password cache.
	void cachePassword(std::string const& _password) const;

	// Only use if previously loaded ok.
	// @returns false if wasn't previously loaded ok.
	bool write() const { return write(m_keysFile); }
	bool write(boost::filesystem::path const& _keysFile) const;
	void write(std::string const& _pass, boost::filesystem::path const& _keysFile) const;	// TODO: all passwords should be a secure string.
	void write(SecureFixedHash<16> const& _key, boost::filesystem::path const& _keysFile) const;

	// Ethereum keys.

	/// Mapping key uuid -> address.
	std::unordered_map<h128, Address> m_uuidLookup;
	/// Mapping address -> key uuid.
	std::unordered_map<Address, h128> m_addrLookup;
	/// Mapping address -> key info.
	std::unordered_map<Address, KeyInfo> m_keyInfo;
	/// Mapping password hash -> password hint.
	std::unordered_map<h256, std::string> m_passwordHint;

	// Passwords that we're storing. Mapping password hash -> password.
	mutable std::unordered_map<h256, std::string> m_cachedPasswords;

	// DEPRECATED.
	// Used to be the default password for keys in the keystore, stored in the keys file.
	// Now the default password is based off the key of the keys file directly, so this is redundant
	// except for the fact that people have existing keys stored with it. Leave for now until/unless
	// we have an upgrade strategy.
	std::string m_defaultPasswordDeprecated;

	mutable boost::filesystem::path m_keysFile;
	mutable SecureFixedHash<16> m_keysFileKey;
	mutable h256 m_master;
	SecretStore m_store;
};

}
}
