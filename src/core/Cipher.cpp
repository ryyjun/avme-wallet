// ================================================================
// Description: Cipher class.
// Copyright:   Copyright (c) 2012 by Joe Linoff
// Version:     1.3.0
// Author:      Joe Linoff
//
// LICENSE
//   The cipher package is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of the
//   License, or (at your option) any later version.
//
//   The cipher package is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details. You should have received
//   a copy of the GNU General Public License along with the change
//   tool; if not, write to the Free Software Foundation, Inc., 59
//   Temple Place, Suite 330, Boston, MA 02111-1307 USA.
// ================================================================
#include "Cipher.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstring>        // strlen
#include <cstdlib>        // getenv
#include <unistd.h>       // getdomainname
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
using namespace std;

// ================================================================
// MACROS
// ================================================================
#define DBG_PRE __FILE__ << ":" << __LINE__ << ": "
#define DBG_FCT(fct)    if(m_debug) cout << DBG_PRE << "FCT " << fct << endl
#define DBG_TDUMP(v)    if(m_debug) tdump(__FILE__, __LINE__, #v, v)
#define DBG_PKV(v)      if(m_debug) vdump(__FILE__, __LINE__, #v, v)
#define DBG_PKVR(k, v)  if(m_debug) vdump(__FILE__, __LINE__, k, v)
#define DBG_BDUMP(a, x) if(m_debug) bdump(__FILE__, __LINE__, #a, a, x)
#define DBG_MDUMP(a)    if(m_debug) bdump(__FILE__, __LINE__, #a, (unsigned char*)a.c_str(), a.size())
#define DBG_MADEIT       cout << DBG_PRE << "MADE IT" << endl
#define PKV(v)           vdump(__FILE__, __LINE__, #v, v)

#define SALTED_PREFIX    "Salted__"

namespace
{
  // ================================================================
  // DEBUG mode only.
  // Formated dump of a general type.
  // ================================================================
  template<typename T> void vdump(const string& fn,
                                  unsigned int ln,
                                  const string& prefix,
                                  const T& d)
  {
    cout << fn << ":" << ln << ": " << prefix << "\t" << d << endl;
  }
  // ================================================================
  // DEBUG mode only.
  // Explicit template instantiation of the above for string
  // types so that I can report the length.
  // ================================================================
  template<> void vdump<string>(const string& fn,
                      unsigned int ln,
                      const string& prefix,
                      const string& d)
  {
    cout << fn << ":" << ln << ": "
         << prefix << "\t"
         << left << setw(64) << d
         << " (" << d.size() << ")"
         << endl;
  }
  // ================================================================
  // DEBUG mode only.
  // Dump for fixed sized types like m_salt and m_key.
  // ================================================================
  template<typename T> void tdump(const string& fn,
                                  unsigned int ln,
                                  const string& prefix,
                                  const T& d)
  {
    cout << fn << ":" << ln << ": " << prefix << "\t";
    for(unsigned int i=0;i<sizeof(T);++i) {
#if 0
      // Prettified output.
      // I turned it off so that the format would match openssl.
      if ((i%16)==0) {
        if (i) {
          cout << endl;
          cout << "\t\t\t";
        }
        else {
          if (prefix.size()<4) {
            cout << "\t";
          }
          cout << "\t";
        }
      }
      else if (i) {
        cout << ", ";
      }
#endif
      unsigned int to_print(d[i]);
      cout << setw(2) << setfill('0') << hex << right << to_print << dec << setfill(' ');
    }
    cout << " (" << sizeof(T) << ")" << endl;
  }
  // ================================================================
  // DEBUG mode only.
  // Binary data dump.
  // ================================================================
  void bdump(const string& fn,
             unsigned int ln,
             const string& prefix,
             unsigned char* a,
             unsigned int len)
  {
    cout << fn << ":" << ln << ": " << prefix;
    for(unsigned int i=0;i<len;++i) {
      if ((i%16)==0) {
        if (i) {
          cout << endl;
          cout << "\t\t\t";
        }
        else {
          cout << "\t\t";
        }
      }
      else if (i) {
        cout << ", ";
      }
      unsigned int to_print(a[i]);
      cout << setw(2) << hex << right << to_print << dec;
    }
    cout << " (" << len << ")" << endl;
  }
}

// ================================================================
// Constructor.
// ================================================================
Cipher::Cipher()
  : m_cipher(CIPHER_DEFAULT_CIPHER),
    m_digest(CIPHER_DEFAULT_DIGEST),
    m_count(CIPHER_DEFAULT_COUNT),
    m_embed(true), // compatible with openssl
    m_debug(false)
{
}

// ================================================================
// Constructor.
// ================================================================
Cipher::Cipher(const std::string& cipher,
	       const std::string& digest,
	       unsigned int count,
	       bool embed)
  : m_cipher(cipher),
    m_digest(digest),
    m_count(count),
    m_embed(embed),
    m_debug(false)
{
}

// ================================================================
// Destructor.
// ================================================================
Cipher::~Cipher()
{
}

// ================================================================
// encrypt
// ================================================================
string Cipher::encrypt(const string& plaintext,
		       const string& pass,
		       const string& salt)
{
  DBG_FCT("encrypt");
  set_salt(salt);
  init(pass);
  kv1_t  x     = encode_cipher(plaintext);
  uchar* ct    = x.first;
  unsigned int   ctlen = x.second;
  DBG_BDUMP(ct, ctlen);

  string ret = encode_base64(ct, ctlen);
  delete [] ct;
  DBG_MDUMP(ret);
  return ret;
}

// ================================================================
// encrypt_file
// ================================================================
void Cipher::encrypt_file(const string& ifn,
			  const string& ofn,
			  const string& pass,
			  const string& salt)
{
  DBG_FCT("encrypt_file");
  string plaintext = file_read(ifn);
  string ciphertext = encrypt(plaintext, pass, salt);
  file_write(ofn, ciphertext, true);
}

// ================================================================
// decrypt
// ================================================================
string Cipher::decrypt(const string& mimetext,
		       const string& pass,
		       const string& salt)
{
  DBG_FCT("decrypt");
  kv1_t  x     = decode_base64(mimetext);
  uchar* ct    = x.first;
  uchar* ctbeg = ct;
  unsigned int   ctlen = x.second;
  DBG_BDUMP(ct, ctlen);

  if (strncmp((const char*)ct, SALTED_PREFIX, 8) == 0) {
    memcpy(m_salt, &ct[8], 8);
    ct += 16;
    ctlen -= 16;
  }
  else {
    set_salt(salt);
  }
  init(pass);
  string ret = decode_cipher(ct, ctlen);
  delete [] ctbeg;
  DBG_MDUMP(ret);
  return ret;
}

// ================================================================
// decrypt_file
// ================================================================
void Cipher::decrypt_file(const string& ifn,
			  const string& ofn,
			  const string& pass,
			  const string& salt)
{
  DBG_FCT("decrypt_file");
  string ciphertext = file_read(ifn);
  string plaintext = decrypt(ciphertext, pass, salt);
  file_write(ofn, plaintext);
}

// ================================================================
// encode_base64
// ================================================================
string Cipher::encode_base64(uchar* ciphertext,
			     unsigned int   ciphertext_len) const
{
  DBG_FCT("encode_base64");
  BIO* b64 = BIO_new(BIO_f_base64());
  BIO* bm  = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bm);
  if (BIO_write(b64, ciphertext, ciphertext_len)<2) {
    throw runtime_error("BIO_write() failed");
  }
  if (BIO_flush(b64)<1) {
    throw runtime_error("BIO_flush() failed");
  }
  BUF_MEM *bptr=0;
  BIO_get_mem_ptr(b64, &bptr);
  unsigned int len=bptr->length;
  char* mimetext = new char[len+1];
  memcpy(mimetext, bptr->data, bptr->length-1);
  mimetext[bptr->length-1]=0;
  BIO_free_all(b64);

  string ret = mimetext;
  delete [] mimetext;
  return ret;
}

// ================================================================
// decode_base64
// ================================================================
Cipher::kv1_t Cipher::decode_base64(const string& mimetext) const
{
  DBG_FCT("decode_base64");
  kv1_t x;
  int SZ=mimetext.size(); // this will always be smaller
  x.first = new uchar[SZ];
  char* tmpbuf = new char[SZ+1];
  strcpy(tmpbuf, mimetext.c_str());
  BIO* b64 = BIO_new(BIO_f_base64());

  // This patch was suggested by Mihai Todor.
  // It was added to the code on 2013-11-21.
  // Please see this post for details:
  //    http://joelinoff.com/blog/?p=664
  if (SZ <= 64) {
    // If the string is less len 64 or less,
    // then the -A switch must be used in
    // openssl.
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  }

  BIO* bm  = BIO_new_mem_buf(tmpbuf, mimetext.size());
  bm = BIO_push(b64, bm);
  x.second = BIO_read(bm, x.first, SZ);
  BIO_free_all(bm);
  delete [] tmpbuf;
  return x;
}

// ================================================================
// encode_cipher
// ================================================================
Cipher::kv1_t Cipher::encode_cipher(const string& plaintext) const
{
  DBG_FCT("encode_cipher");
  unsigned int SZ = plaintext.size() + AES_BLOCK_SIZE + 20;  // leave some padding
  uchar* ciphertext = new uchar[SZ];
  memset((ciphertext), '\0', (SZ));
  uchar* pbeg = ciphertext;

  // This requires some explanation.
  // In order to be compatible with openssl, I need to append
  // 16 characters worth of information that describe the salt.
  // I found this in the openssl source code but I couldn't
  // find any associated documentation.
  unsigned int off = 0;
  if (m_embed) {
    memcpy(&ciphertext[0], SALTED_PREFIX, 8);
    memcpy(&ciphertext[8], m_salt, 8);
    off = 16;
    ciphertext += off;
  }

  int ciphertext_len=0;
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER* cipher = EVP_aes_256_cbc();
  EVP_CIPHER_CTX_init(ctx);
  if (1 != EVP_EncryptInit_ex(ctx, cipher, NULL, m_key, m_iv)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_EncryptInit_ex() init key/iv failed");
  }
  EVP_CIPHER_CTX_set_key_length(ctx, EVP_MAX_KEY_LENGTH);

  // Encrypt the plaintext data all at once.
  // It would be straightforward to chunk it but that
  // add unecesary complexity at this point.
  uchar* pt_buf = (uchar*)plaintext.c_str();
  unsigned int   pt_len = plaintext.size();
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, pt_buf, pt_len)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_EncryptUpdate() failed");
  }

  uchar* pad_buf = ciphertext + ciphertext_len; // pad at the end
  int pad_len=0;
  if (1 != EVP_EncryptFinal_ex(ctx, pad_buf, &pad_len)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_EncryptFinal_ex() failed");
  }

  ciphertext_len += pad_len + off; // <off> for the Salted prefix
  EVP_CIPHER_CTX_free(ctx);
  return kv1_t(pbeg, ciphertext_len);
}

// ================================================================
// decode_cipher
// ================================================================
string Cipher::decode_cipher(uchar* ciphertext,
			     unsigned int   ciphertext_len) const
{
  DBG_FCT("decode_cipher");
  const unsigned int SZ = ciphertext_len+20;
  uchar* plaintext = new uchar[SZ];
  int plaintext_len = 0;
  const EVP_CIPHER* cipher = EVP_aes_256_cbc();
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  memset((plaintext), '\0', (SZ));
  EVP_CIPHER_CTX_init(ctx);

  if (1 != EVP_DecryptInit_ex(ctx, cipher, NULL, m_key, m_iv)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_DecryptInit_ex() failed");
  }
  EVP_CIPHER_CTX_set_key_length(ctx, EVP_MAX_KEY_LENGTH);

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, ciphertext, ciphertext_len)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_DecryptUpdate() failed");
  }

  int plaintext_padlen=0;
  if (1 != EVP_DecryptFinal_ex(ctx, plaintext+plaintext_len, &plaintext_padlen)) {
    EVP_CIPHER_CTX_free(ctx);
    throw runtime_error("EVP_DecryptFinal_ex() failed");
  }
  plaintext_len += plaintext_padlen;
  plaintext[plaintext_len] = 0;

  string ret = (char*)plaintext;
  delete [] plaintext;
  EVP_CIPHER_CTX_free(ctx);
  return ret;
}

// ================================================================
// set_salt
// ================================================================
void Cipher::set_salt(const string& salt)
{
  DBG_FCT("set_salt");
  if (salt.length() == 0) {
    // Choose a random salt.
    for(unsigned int i=0;i<sizeof(m_salt);++i) {
      m_salt[i] = rand() % 256;
    }
  }
  else if (salt.length() == 8) {
    memcpy(m_salt, salt.c_str(), 8);
  }
  else if (salt.length()<8) {
    throw underflow_error("init(): salt is too short, must be 8 characters");
  }
  else if (salt.length()>8) {
    throw overflow_error("init(): salt is too long, must be 8 characters");
  }
}

// ================================================================
// init()
// ================================================================
void Cipher::init(const string& pass)
{
  DBG_FCT("init");

  // Use a default passphrase if the user didn't specify one.
  m_pass = pass;
  if (m_pass.empty() ) {
    // Default: ' deFau1t pASsw0rD'
    // Obfuscate so that a simple strings will not find it.
    char a[] = {' ', 'd', 'e', 'F', 'a', 'u', '1', 't', ' ',
		'p', 'A', 'S', 's', 'w', '0', 'r', 'D', 0};
    m_pass = a;
  }

  // Create the key and IV values from the passkey.
  memset((m_key), '\0', (sizeof(m_key)));
  memset((m_iv), '\0', (sizeof(m_iv)));
  OpenSSL_add_all_algorithms();
  const EVP_CIPHER* cipher = EVP_get_cipherbyname(m_cipher.c_str());
  const EVP_MD*     digest = EVP_get_digestbyname(m_digest.c_str());
  if (!cipher) {
    string msg = "init(): cipher does not exist "+m_cipher;
    throw runtime_error(msg);
  }
  if (!digest) {
    string msg = "init(): digest does not exist "+m_digest;
    throw runtime_error(msg);
  }

  int ks = EVP_BytesToKey(cipher,    // cipher type
			  digest,    // message digest
			  m_salt,    // 8 bytes
			  (uchar*)m_pass.c_str(), // pass value
			  m_pass.length(),
			  m_count,   // number of rounds
			  m_key,
			  m_iv);
  if (ks!=32) {
    throw runtime_error("init() failed: "
			"EVP_BytesToKey did not return a 32 byte key");
  }

  DBG_PKV(m_pass);
  DBG_PKV(m_cipher);
  DBG_PKV(m_digest);
  DBG_TDUMP(m_salt);
  DBG_TDUMP(m_key);
  DBG_TDUMP(m_iv);
  DBG_PKV(m_count);
}

// ================================================================
// file_read
// ================================================================
string Cipher::file_read(const string& fn) const
{
  DBG_FCT("file_read");
  ifstream ifs(fn.c_str());
  if (!ifs) {
    string msg="Cannot read file '"+fn+"'";
    throw runtime_error(msg);
  }
  string str((istreambuf_iterator<char>(ifs)),
	     istreambuf_iterator<char>());
  return str;
}

// ================================================================
// file_write
// ================================================================
void Cipher::file_write(const string& fn, const string& data, bool nl) const
{
  DBG_FCT("file_write");
  ofstream ofs(fn.c_str());
  if (!ofs) {
    string msg="Cannot write file '"+fn+"'";
    throw runtime_error(msg);
  }
  ofs << data;
  if (nl) {
    ofs << endl;
  }
  ofs.close();
}

// ================================================================
// get_version
// ================================================================
std::string Cipher::get_version() {
  return "1.3.0";
}

// ================================================================
// get_ssl_version
// ================================================================
#define TO_STRING_(x) #x
#define TO_STRING(x) TO_STRING_(x)
std::string Cipher::get_ssl_version() {
  return TO_STRING(OPENSSL_VERSION_NUMBER);
}
