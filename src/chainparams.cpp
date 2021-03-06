// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Copyright (c) 2019 The BitGreen Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <chainparams.h>
#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <util/system.h>
#include <versionbitsinfo.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.nType = 0;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=18d67153a6109201bd1fa74d9ff94785d31a83cd0d0cda00af5d8ea79beca1bd, ver=0x00000001, hashPrevBlock=0000000000000000000000000000000000000000000000000000000000000000, hashMerkleRoot=07cbcacfc822fba6bbeb05312258fa43b96a68fc310af8dfcec604591763f7cf, nTime=1565017975, nBits=1e0ffff0, nNonce=21212214, vtx=1)
 *  CTransaction(hash=07cbcacfc8, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *    CTxIn(COutPoint(0000000000, 4294967295), coinbase 04ffff001d01044c554576656e205769746820456e6572677920537572706c75732c2043616e61646120556e61626c6520746f204d65657420456c6563747269636974792044656d616e6473206f6620426974636f696e204d696e657273)
 *    CScriptWitness()
 *    CTxOut(nValue=0.00000000, scriptPubKey=4104e5a8143f86ad8ac63791fbbdb8)
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Even With Energy Surplus, Canada Unable to Meet Electricity Demands of Bitcoin Miners";
    const CScript genesisOutputScript = CScript() << ParseHex("04e5a8143f86ad8ac63791fbbdb8e0b91a8da88c8c693a95f6c2c13c063ea790f7960b8025a9047a7bc671d5cfe707a2dd2e13b86182e1064a0eea7bf863636363") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

// this one is for testing only
static Consensus::LLMQParams llmq5_60 = {
    .type = Consensus::LLMQ_5_60,
    .name = "llmq_5_60",
    .size = 5,
    .minSize = 3,
    .threshold = 3,

    .dkgInterval = 24, // one DKG per hour
    .dkgPhaseBlocks = 2,
    .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
    .dkgMiningWindowEnd = 18,
    .dkgBadVotesThreshold = 8,

    .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

    .keepOldConnections = 3,
};

static Consensus::LLMQParams llmq50_60 = {
    .type = Consensus::LLMQ_50_60,
    .name = "llmq_50_60",
    .size = 50,
    .minSize = 40,
    .threshold = 30,

    .dkgInterval = 24, // one DKG per hour
    .dkgPhaseBlocks = 2,
    .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
    .dkgMiningWindowEnd = 18,
    .dkgBadVotesThreshold = 40,

    .signingActiveQuorumCount = 24, // a full day worth of LLMQs

    .keepOldConnections = 25,
};

static Consensus::LLMQParams llmq400_60 = {
    .type = Consensus::LLMQ_400_60,
    .name = "llmq_400_60",
    .size = 400,
    .minSize = 300,
    .threshold = 240,

    .dkgInterval = 24 * 12, // one DKG every 12 hours
    .dkgPhaseBlocks = 4,
    .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
    .dkgMiningWindowEnd = 28,
    .dkgBadVotesThreshold = 300,

    .signingActiveQuorumCount = 4, // two days worth of LLMQs

    .keepOldConnections = 5,
};

// Used for deployment and min-proto-version signalling, so it needs a higher threshold
static Consensus::LLMQParams llmq400_85 = {
    .type = Consensus::LLMQ_400_85,
    .name = "llmq_400_85",
    .size = 400,
    .minSize = 350,
    .threshold = 340,

    .dkgInterval = 24 * 24, // one DKG every 24 hours
    .dkgPhaseBlocks = 4,
    .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
    .dkgMiningWindowEnd = 48,   // give it a larger mining window to make sure it is mined
    .dkgBadVotesThreshold = 300,

    .signingActiveQuorumCount = 4, // two days worth of LLMQs

    .keepOldConnections = 5,
};

/**
 * Main network
 */
class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 525600;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 20;
        consensus.nPowTargetTimespan = 1 * 24 * 60 * 60;                                                   // 1 day
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPosTargetSpacing = 2 * 60; // PoS: 2 minutes
        consensus.nPosTargetTimespan = 60 * 40;
        consensus.nModifierInterval = 60;      // Modifier interval: time to elapse before new modifier is computed (60 seconds)
        consensus.nLastPoWBlock = 1500;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016;       // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMasternodeMinimumConfirmations = 15;

        // Stake constants
        consensus.nStakeEnforcement = 70000;
        consensus.nMinStakeHistory = 360;
        consensus.minAgeDefinitions = {{ {      0,    60*60*12 },
                                         { 175000,    60*60*24 } }};
        consensus.maxAgeDefinitions = {{ {      0,    60*60*48 },
                                         { 175000,    60*60*96 } }};
        consensus.heightDefinitions = {{ {  70000,  200 * COIN } }};
        consensus.weightDefinitions = {{ {      0,         200 },
                                         { 175000,        1000 } }};

        // Governance
        consensus.nSuperblockCycle = 20571; // ~(60*24*30)/2.1, actual number of blocks per month is 262800 / 12 = 21900
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nBudgetPaymentsStartBlock = 10000;
        consensus.nBudgetPaymentsCycleBlocks = 20571; // ~(60*24*30)/2.1, actual number of blocks per month is 262800 / 12 = 21900
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nSuperblockStartBlock = 12000; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPaymentsStartBlock

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;   // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800;   // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");
        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // InstantSend
        consensus.nInstantSendConfirmationsRequired = 6;
        consensus.nInstantSendKeepLock = 24;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xe4;
        pchMessageStart[1] = 0xa4;
        pchMessageStart[2] = 0x06;
        pchMessageStart[3] = 0x1f;
        nDefaultPort = 9333;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 0;

        genesis = CreateGenesisBlock(1574334000, 27296764, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000025289d6b03cbda4950e825cd865185f34fbb3e098295534b63d78beba15"));
        assert(genesis.hashMerkleRoot == uint256S("0x07cbcacfc822fba6bbeb05312258fa43b96a68fc310af8dfcec604591763f7cf"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 6);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 46);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "bg";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqChainLocks = Consensus::LLMQ_400_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_50_60;
        consensus.nLLMQActivationHeight = 50;

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        fMiningRequiresPeers = true;
        fAllowMultiplePorts = true;
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour

        vSporkAddresses = {"GMWbuDW6m6WCc7Zc9W3CSuviXzqPKK3eBj"};
        nMinSporkKeys = 1;

        checkpointData = {
            {
                {      1, uint256S("0x0000062cf9ac97b1582474e313770e4609c338ed6fae01142da65722353465f3")},
                {    100, uint256S("0x000005faf4d7d9dccd3a1986eb7150a22f21f80664d5deb91cb1ca38eb305e7e")},
                {   6439, uint256S("0x7c6f9621fe18f22e57d042a3804be45a9ace2d17a305036242d7ba90b68345cb")},
                {  70004, uint256S("0x2da7cf773e5032a76aa4480b033c1ac6978ff64531f168c92d022c90f5bf7996")},
                {  80000, uint256S("0x1f6545f0cd4a07a02a5b0175f22b371fc1839839df8d835c04f6420a08d43877")},
                {  90000, uint256S("0x1d4a1b059b96fa871e9aa09eca0e2ed18ef369556ef8ee88bacf3b3705812e26")},
                { 100000, uint256S("0x8a58bc2b0d6b13229f4ec1d9733317a82e62dbc035e09384ee9e73b77a3e3c76")},
                { 105000, uint256S("0xa9e075e368ebc428c223055d4c3db108106098237dc9f55af687f56781c4d932")},
                { 110000, uint256S("0xfc62dddbd615c0c5d34fc24cb4f6d6b86f02465c036ac42d1bda585e1ac3d066")}
            }};

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats <nblock> <blockhash>
            // Data from RPC: getchaintxstats 70004 2da7cf773e5032a76aa4480b033c1ac6978ff64531f168c92d022c90f5bf7996
            /* nTime    */ 1583583293,
            /* nTxCount */ 268247,
            /* dTxRate  */ 0.02924633374616526};
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams
{
public:
    CTestNetParams()
    {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 200;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 200;
        consensus.BIP66Height = 200;
        consensus.powLimit = uint256S("00000ffff0000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nPosTargetSpacing = 2 * 60; // PoS: 2 minutes
        consensus.nPosTargetTimespan = 60 * 40;
        consensus.nModifierInterval = 60; // Modifier interval: time to elapse before new modifier is computed (1 minute)
        consensus.nLastPoWBlock = 200;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016;       // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMasternodeMinimumConfirmations = 1;

        // Stake constants
        consensus.nStakeEnforcement = 200;
        consensus.nMinStakeHistory = 10;
        consensus.heightDefinitions = {{ {    200, 1 * COIN } }};

        // Governance
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nBudgetPaymentsStartBlock = 200;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 300; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPaymentsStartBlock

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;   // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800;   // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800;   // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // InstantSend
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;

        pchMessageStart[0] = 0xa3;
        pchMessageStart[1] = 0x6b;
        pchMessageStart[2] = 0xb0;
        pchMessageStart[3] = 0x4b;
        nDefaultPort = 19333;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 0;

        genesis = CreateGenesisBlock(1565017975, 21212214, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000546a6b03a54ae05f94119e37c55202e90a953058c35364d112d41ded06a"));
        assert(genesis.hashMerkleRoot == uint256S("0x07cbcacfc822fba6bbeb05312258fa43b96a68fc310af8dfcec604591763f7cf"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 98);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 12);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 108);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tbg";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqChainLocks = Consensus::LLMQ_50_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_50_60;
        consensus.nLLMQActivationHeight = 50;

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        fMiningRequiresPeers = true;
        fAllowMultiplePorts = false;
        nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"gprpehZBigGDp7sNMjEKY46afAd9BWtd29"};
        nMinSporkKeys = 1;

        checkpointData = {
            {}};

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats <nblocks> <hash>
            /* nTime    */ 0,
            /* nTxCount */ 0,
            /* dTxRate  */ 0};
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const ArgsManager& args)
    {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = consensus.nLastPoWBlock; // BIP34 activated on regtest (Used in functional tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = consensus.nLastPoWBlock; // BIP65 activated on regtest (Used in functional tests)
        consensus.BIP66Height = consensus.nLastPoWBlock; // BIP66 activated on regtest (Used in functional tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPosTargetSpacing = 2 * 60; // PoS: 2 minutes
        consensus.nPosTargetTimespan = 60 * 40;
        consensus.nModifierInterval = 60; // Modifier interval: time to elapse before new modifier is computed (1 minute)
        consensus.nLastPoWBlock = 1000;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144;       // Faster than normal for regtest (144 instead of 2016)
        consensus.nMasternodeMinimumConfirmations = 1;

        //! stake constants
        consensus.heightDefinitions = {{ {    200, 1 * COIN } }};

        // Governance
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 1500; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPaymentsStartBlock

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // InstantSend
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;

        pchMessageStart[0] = 0xf2;
        pchMessageStart[1] = 0x90;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0x78;
        nDefaultPort = 29333;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateVersionBitsParametersFromArgs(args);

        genesis = CreateGenesisBlock(1565017975, 20542302, 0x207fffff, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x100a3271b95d1a817101bcbd7045ad14c9799cb34e1cb6071973c8932ae48b6a"));
        assert(genesis.hashMerkleRoot == uint256S("0x07cbcacfc822fba6bbeb05312258fa43b96a68fc310af8dfcec604591763f7cf"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = true;
        m_is_test_chain = true;
        fMiningRequiresPeers = false;
        fAllowMultiplePorts = true;
        nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"gprpehZBigGDp7sNMjEKY46afAd9BWtd29"};
        nMinSporkKeys = 1;

        checkpointData = {
            {}};

        chainTxData = ChainTxData{
            0,
            0,
            0};

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 98);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 12);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 108);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "bgrt";

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_5_60] = llmq5_60;
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqChainLocks = Consensus::LLMQ_5_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_5_60;
        consensus.nLLMQActivationHeight = 500;
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
    void UpdateVersionBitsParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateVersionBitsParametersFromArgs(const ArgsManager& args)
{
    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() != 3) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end");
        }
        int64_t nStartTime, nTimeout;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        bool found = false;
        for (int j = 0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld\n", vDeploymentParams[0], nStartTime, nTimeout);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams& Params()
{
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams(gArgs));
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}
