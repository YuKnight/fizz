/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/AESOCB128.h>
#include <fizz/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/exchange/ECCurveKeyExchange.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/crypto/exchange/X25519.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/HandshakeContext.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/record/PlaintextRecordLayer.h>
#include <fizz/record/Types.h>

namespace fizz {

/**
 * This class instantiates various objects to facilitate testing.
 */
class Factory {
 public:
  virtual ~Factory() = default;

  virtual std::unique_ptr<PlaintextReadRecordLayer>
  makePlaintextReadRecordLayer() const {
    return std::make_unique<PlaintextReadRecordLayer>();
  }

  virtual std::unique_ptr<PlaintextWriteRecordLayer>
  makePlaintextWriteRecordLayer() const {
    return std::make_unique<PlaintextWriteRecordLayer>();
  }

  virtual std::unique_ptr<EncryptedReadRecordLayer>
  makeEncryptedReadRecordLayer() const {
    return std::make_unique<EncryptedReadRecordLayer>();
  }

  virtual std::unique_ptr<EncryptedWriteRecordLayer>
  makeEncryptedWriteRecordLayer() const {
    return std::make_unique<EncryptedWriteRecordLayer>();
  }

  virtual std::unique_ptr<KeyScheduler> makeKeyScheduler(
      CipherSuite cipher) const {
    auto keyDer = makeKeyDeriver(cipher);
    return std::make_unique<KeyScheduler>(std::move(keyDer));
  }

  virtual std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const {
    switch (cipher) {
      case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      case CipherSuite::TLS_AES_128_GCM_SHA256:
      case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
        return std::make_unique<KeyDerivationImpl<Sha256>>(getHkdfPrefix());
      case CipherSuite::TLS_AES_256_GCM_SHA384:
        return std::make_unique<KeyDerivationImpl<Sha384>>(getHkdfPrefix());
      default:
        throw std::runtime_error("ks: not implemented");
    }
  }

  virtual std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const {
    switch (cipher) {
      case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
      case CipherSuite::TLS_AES_128_GCM_SHA256:
      case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
        return std::make_unique<HandshakeContextImpl<Sha256>>(getHkdfPrefix());
      case CipherSuite::TLS_AES_256_GCM_SHA384:
        return std::make_unique<HandshakeContextImpl<Sha384>>(getHkdfPrefix());
      default:
        throw std::runtime_error("hs: not implemented");
    }
  }

  virtual std::unique_ptr<KeyExchange> makeKeyExchange(NamedGroup group) const {
    switch (group) {
      case NamedGroup::secp256r1:
        return std::make_unique<OpenSSLKeyExchange<P256>>();
      case NamedGroup::secp384r1:
        return std::make_unique<OpenSSLKeyExchange<P384>>();
      case NamedGroup::secp521r1:
        return std::make_unique<OpenSSLKeyExchange<P521>>();
      case NamedGroup::x25519:
        return std::make_unique<X25519KeyExchange>();
      default:
        throw std::runtime_error("ke: not implemented");
    }
  }

  virtual std::unique_ptr<Aead> makeAead(CipherSuite cipher) const {
    switch (cipher) {
      case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
        return std::make_unique<OpenSSLEVPCipher<ChaCha20Poly1305>>();
      case CipherSuite::TLS_AES_128_GCM_SHA256:
        return std::make_unique<OpenSSLEVPCipher<AESGCM128>>();
      case CipherSuite::TLS_AES_256_GCM_SHA384:
        return std::make_unique<OpenSSLEVPCipher<AESGCM256>>();
      case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
        return std::make_unique<OpenSSLEVPCipher<AESOCB128>>();
      default:
        throw std::runtime_error("aead: not implemented");
    }
  }

  virtual Random makeRandom() const {
    return RandomGenerator<Random().size()>().generateRandom();
  }

  virtual uint32_t makeTicketAgeAdd() const {
    return RandomNumGenerator<uint32_t>().generateRandom();
  }

  virtual std::shared_ptr<PeerCert> makePeerCert(Buf certData) const {
    return CertUtils::makePeerCert(std::move(certData));
  }

  virtual std::string getHkdfPrefix() const {
    return kHkdfLabelPrefix.str();
  }
};
} // namespace fizz
