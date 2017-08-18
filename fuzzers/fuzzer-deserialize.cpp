#include "fuzzer.h"
#include "consensus/merkle.h"
#include "primitives/block.h"
#include "script/script.h"
#include "addrman.h"
#include "chain.h"
#include "coins.h"
#include "compressor.h"
#include "net.h"
#include "protocol.h"
#include "streams.h"
#include "undo.h"
#include "version.h"
#include "pubkey.h"

#include <stdint.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

/* Fuzzers the deserialization functions of various classes
 * Taken from src/test/test_bitcoin_fuzzy.cpp
 */

enum TEST_ID {
    CBLOCK_DESERIALIZE=0,
    CTRANSACTION_DESERIALIZE,
    CBLOCKLOCATOR_DESERIALIZE,
    CBLOCKMERKLEROOT,
    CADDRMAN_DESERIALIZE,
    CBLOCKHEADER_DESERIALIZE,
    CBANENTRY_DESERIALIZE,
    CTXUNDO_DESERIALIZE,
    CBLOCKUNDO_DESERIALIZE,
    CCOINS_DESERIALIZE,
    CNETADDR_DESERIALIZE,
    CSERVICE_DESERIALIZE,
    CMESSAGEHEADER_DESERIALIZE,
    CADDRESS_DESERIALIZE,
    CINV_DESERIALIZE,
    CBLOOMFILTER_DESERIALIZE,
    CDISKBLOCKINDEX_DESERIALIZE,
    CTXOUTCOMPRESSOR_DESERIALIZE,
    CEXTKEY_DESERIALIZE,
    CEXTPUBKEY_DESERIALIZE,
    UINT256_DESERIALIZE,
    TEST_ID_END
};

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    ECCVerifyHandle globalVerifyHandle;
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    std::vector<uint8_t> buffer;
    buffer.assign(data, data+size);

    if (buffer.size() < sizeof(uint32_t)) return 0;

    uint32_t test_id = 0xffffffff;
    memcpy(&test_id, &buffer[0], sizeof(uint32_t));
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t));

    if (test_id >= TEST_ID_END) return 0;

    CDataStream ds(buffer, SER_NETWORK, INIT_PROTO_VERSION);
    try {
        int nVersion;
        ds >> nVersion;
        ds.SetVersion(nVersion);
    } catch (const std::ios_base::failure& e) {
        return 0;
    }

    switch(test_id) {
        case CBLOCK_DESERIALIZE:
        {
            try
            {
                CBlock block;
                ds >> block;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTRANSACTION_DESERIALIZE:
        {
            try
            {
                CTransaction tx(deserialize, ds);
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKLOCATOR_DESERIALIZE:
        {
            try
            {
                CBlockLocator bl;
                ds >> bl;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKMERKLEROOT:
        {
            try
            {
                CBlock block;
                ds >> block;
                bool mutated;
                BlockMerkleRoot(block, &mutated);
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CADDRMAN_DESERIALIZE:
        {
            try
            {
                CAddrMan am;
                ds >> am;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKHEADER_DESERIALIZE:
        {
            try
            {
                CBlockHeader bh;
                ds >> bh;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBANENTRY_DESERIALIZE:
        {
            try
            {
                CBanEntry be;
                ds >> be;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTXUNDO_DESERIALIZE:
        {
            try
            {
                CTxUndo tu;
                ds >> tu;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKUNDO_DESERIALIZE:
        {
            try
            {
                CBlockUndo bu;
                ds >> bu;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CCOINS_DESERIALIZE:
        {
            try
            {
                CCoins block;
                ds >> block;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CNETADDR_DESERIALIZE:
        {
            try
            {
                CNetAddr na;
                ds >> na;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CSERVICE_DESERIALIZE:
        {
            try
            {
                CService s;
                ds >> s;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CMESSAGEHEADER_DESERIALIZE:
        {
            CMessageHeader::MessageStartChars pchMessageStart = {0x00, 0x00, 0x00, 0x00};
            try
            {
                CMessageHeader mh(pchMessageStart);
                ds >> mh;
                if (!mh.IsValid(pchMessageStart)) {return 0;}
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CADDRESS_DESERIALIZE:
        {
            try
            {
                CAddress a;
                ds >> a;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CINV_DESERIALIZE:
        {
            try
            {
                CInv i;
                ds >> i;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOOMFILTER_DESERIALIZE:
        {
            try
            {
                CBloomFilter bf;
                ds >> bf;
                std::vector<unsigned char> vec1;
                vec1.resize(100);
                memset(vec1.data(), 0xAA, 100);
                bf.insert(vec1);
                bf.contains(vec1);
                bf.clear();
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CDISKBLOCKINDEX_DESERIALIZE:
        {
            try
            {
                CDiskBlockIndex dbi;
                ds >> dbi;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTXOUTCOMPRESSOR_DESERIALIZE:
        {
            CTxOut to;
            CTxOutCompressor toc(to);
            try
            {
                ds >> toc;
            } catch (const std::ios_base::failure& e) {return 0;}

            break;
        }
        case CEXTKEY_DESERIALIZE:
        {
            try
            {
                CExtKey ek;
                ds >> ek;
            } catch (const std::ios_base::failure& e) {return 0;}
            /* CExtKey throws runtime_error if key size is invalid */
            catch (const std::runtime_error& e) { return 0; }

            break;
        }
        case CEXTPUBKEY_DESERIALIZE:
        {
            try
            {
                CExtPubKey epk;
                ds >> epk;
            } catch (const std::ios_base::failure& e) {return 0;}
            /* CExtPubKey throws runtime_error if key size is invalid */
            catch (const std::runtime_error& e) { return 0; }

            break;
        }
        case UINT256_DESERIALIZE:
        {
            try
            {
                uint256 u256;
                ds >> u256;
            } catch (const std::ios_base::failure& e) {return 0;}

            break;
        }
        default:
            return 0;
    }
    return 0;
}
