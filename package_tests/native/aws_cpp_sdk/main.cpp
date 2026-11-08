/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_cpp_sdk: core API; calls Aws::InitAPI(), Aws::Utils::HashingUtils::CalculateMD5(), Aws::ShutdownAPI().
 */

#include <aws/core/Aws.h>
#include <aws/core/utils/crypto/Hash.h>
#include <aws/core/utils/crypto/Factories.h>
#include <aws/core/utils/HashingUtils.h>
#include <cassert>
#include <iostream>

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        /* Exercise the SDK's crypto factory — links libcrypto under the hood.
         * Catches a broken openssl wire-up while the SDK is initialized. */
        Aws::Utils::ByteBuffer md5 = Aws::Utils::HashingUtils::CalculateMD5("hello");
        assert(md5.GetLength() == 16);
    }
    Aws::ShutdownAPI(options);

    std::cout << "aws_cpp_sdk test OK" << std::endl;
    return 0;
}
