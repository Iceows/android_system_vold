/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_VOLD_KEYSTORAGE_H
#define ANDROID_VOLD_KEYSTORAGE_H

#include "KeyBuffer.h"

#include <string>

namespace android {
namespace vold {

// Represents the information needed to decrypt a disk encryption key.
// If "token" is nonempty, it is passed in as a required Gatekeeper auth token.
// If "token" and "secret" are nonempty, "secret" is appended to the application-specific
// binary needed to unlock.
// If only "secret" is nonempty, it is used to decrypt in a non-Keymaster process.
class KeyAuthentication {
  public:
    KeyAuthentication(const std::string& t, const std::string& s);

    bool usesKeymaster() const { return !token.empty() || secret.empty(); };

    std::string token;
    std::string secret;
};

extern const KeyAuthentication kEmptyAuthentication;

// Checks if path "path" exists.
bool pathExists(const std::string& path);

bool createSecdiscardable(const std::string& path, std::string* hash);
bool readSecdiscardable(const std::string& path, std::string* hash);

// Create a directory at the named path, and store "key" in it,
// in such a way that it can only be retrieved via Keymaster and
// can be securely deleted.
// It's safe to move/rename the directory after creation.
bool storeKey(const std::string& dir, const KeyAuthentication& auth, const KeyBuffer& key);

// Create a directory at the named path, and store "key" in it as storeKey
// This version creates the key in "tmp_path" then atomically renames "tmp_path"
// to "key_path" thereby ensuring that the key is either stored entirely or
// not at all.
bool storeKeyAtomically(const std::string& key_path, const std::string& tmp_path,
                        const KeyAuthentication& auth, const KeyBuffer& key);

// Retrieve the key from the named directory.
//
// If the key is wrapped by a Keymaster key that requires an upgrade, then that
// Keymaster key is upgraded.  If |keepOld| is false, then the upgraded
// Keymaster key replaces the original one.  As part of this, the original is
// deleted from Keymaster; however, if a user data checkpoint is active, this
// part is delayed until the checkpoint is committed.
//
// If instead |keepOld| is true, then the upgraded key doesn't actually replace
// the original one.  This is needed *only* if |dir| isn't located in /data and
// a user data checkpoint is active.  In this case the caller must handle
// replacing the original key if the checkpoint is committed, and deleting the
// upgraded key if the checkpoint is rolled back.
bool retrieveKey(const std::string& dir, const KeyAuthentication& auth, KeyBuffer* key,
                 bool keepOld);

// Securely destroy the key stored in the named directory and delete the directory.
bool destroyKey(const std::string& dir);

bool runSecdiscardSingle(const std::string& file);

// Generate wrapped storage key using keymaster. Uses STORAGE_KEY tag in keymaster.
bool generateWrappedStorageKey(KeyBuffer* key);
// Export the per-boot boot wrapped storage key using keymaster.
bool exportWrappedStorageKey(const KeyBuffer& kmKey, KeyBuffer* key);
}  // namespace vold
}  // namespace android

#endif
