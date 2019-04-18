/********************************************************************
 * (C) 2019 Michael Toutonghi
 * 
 * Distributed under the MIT software license, see the accompanying
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.
 * 
 * This provides support for PBaaS initialization, notarization, and cross-chain token
 * transactions and enabling liquid or non-liquid tokens across the
 * Verus ecosystem.
 * 
 */

#include "pbaas/pbaas.h"
#include "pbaas/notarization.h"
#include "rpc/pbaasrpc.h"
#include "pbaas/crosschainrpc.h"
#include "base58.h"
#include "timedata.h"

using namespace std;

CConnectedChains ConnectedChains;

bool IsVerusActive()
{
    return (strcmp(ASSETCHAINS_SYMBOL, "VRSC") == 0 || strcmp(ASSETCHAINS_SYMBOL, "VRSCTEST") == 0);
}

// this adds an opret to a mutable transaction and returns the voutnum if it could be added
int32_t AddOpRetOutput(CMutableTransaction &mtx, const CScript &opRetScript)
{
    if (opRetScript.IsOpReturn() && opRetScript.size() <= MAX_OP_RETURN_RELAY)
    {
        CTxOut vOut = CTxOut();
        vOut.scriptPubKey = opRetScript;
        vOut.nValue = 0;
        mtx.vout.push_back(vOut);
        return mtx.vout.size() - 1;
    }
    else
    {
        return -1;
    }
}

// returns a pointer to a base chain object, which can be cast to the
// object type indicated in its objType member
uint256 GetChainObjectHash(const CBaseChainObject &bo)
{
    union {
        const CChainObject<CBlockHeader> *pNewHeader;
        const CChainObject<CTransaction> *pNewTx;
        const CChainObject<CMerkleBranch> *pNewProof;
        const CChainObject<CHeaderRef> *pNewHeaderRef;
        const CChainObject<CTransactionRef> *pNewTxRef;
        const CChainObject<COpRetRef> *pNewOpRetRef;
        const CBaseChainObject *retPtr;
    };

    retPtr = &bo;

    switch(bo.objectType)
    {
        case CHAINOBJ_HEADER:
            return pNewHeader->GetHash();

        case CHAINOBJ_TRANSACTION:
            return pNewTx->GetHash();

        case CHAINOBJ_PROOF:
            return pNewProof->GetHash();

        case CHAINOBJ_HEADER_REF:
            return pNewHeaderRef->GetHash();

        case CHAINOBJ_TRANSACTION_REF:
            return pNewTxRef->GetHash();

        case CHAINOBJ_OPRET_REF:
            return pNewOpRetRef->GetHash();
    }
    return uint256();
}

// used to export coins from one chain to another, if they are not native, they are represented on the other
// chain as tokens
bool ValidateChainExport(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

// used to validate import of coins from one chain to another. if they are not native and are supported,
// they are represented o the chain as tokens
bool ValidateChainImport(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

// used to validate a specific service reward based on the spending transaction
bool ValidateServiceReward(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

// used as a proxy token output for a reserve currency on its fractional reserve chain
bool ValidateReserveOutput(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

// used to convert a fractional reserve currency into its reserve and back 
bool ValidateReserveExchange(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

// used for distribution of premine
bool ValidatePremineOutput(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{

}

/*
 * Verifies that the input objects match the hashes and returns the transaction.
 * 
 * If the opRetTx has the op ret, this calculates based on the actual transaction and
 * validates the hashes. If the opRetTx does not have the opRet itself, this validates
 * by ensuring that all objects are present on this chain, composing the opRet, and
 * ensuring that the transaction then hashes to the correct txid.
 * 
 */
bool ValidateOpretProof(CScript &opRet, COpRetProof &orProof)
{
    // enumerate through the objects and validate that they are objects of the expected type that hash
    // to the value expected. return true if so
        
}

int8_t ObjTypeCode(COpRetProof &obj)
{
    return CHAINOBJ_OPRETPROOF;
}

int8_t ObjTypeCode(CBlockHeader &obj)
{
    return CHAINOBJ_HEADER;
}

int8_t ObjTypeCode(CMerkleBranch &obj)
{
    return CHAINOBJ_PROOF;
}

int8_t ObjTypeCode(CTransaction &obj)
{
    return CHAINOBJ_TRANSACTION;
}

int8_t ObjTypeCode(CHeaderRef &obj)
{
    return CHAINOBJ_HEADER_REF;
}

int8_t ObjTypeCode(CTransactionRef &obj)
{
    return CHAINOBJ_TRANSACTION_REF;
}

int8_t ObjTypeCode(COpRetRef &obj)
{
    return CHAINOBJ_OPRET_REF;
}

// this adds an opret to a mutable transaction that provides the necessary evidence of a signed, cheating stake transaction
CScript StoreOpRetArray(std::vector<CBaseChainObject *> &objPtrs)
{
    CScript vData;
    CDataStream s = CDataStream(SER_NETWORK, PROTOCOL_VERSION);
    bool error = false;

    for (auto pobj : objPtrs)
    {
        if (!DehydrateChainObject(s, pobj))
        {
            error = true;
            break;
        }
    }

    std::vector<unsigned char> vch(s.begin(), s.end());

    vData << OPRETTYPE_OBJECTARR << vch;
    vch = std::vector<unsigned char>(vData.begin(), vData.end());
    return CScript() << OP_RETURN << vch;
}

std::vector<CBaseChainObject *> RetrieveOpRetArray(const CScript &opRetScript)
{
    std::vector<unsigned char> vch;
    std::vector<CBaseChainObject *> vRet;
    if (opRetScript.IsOpReturn() && GetOpReturnData(opRetScript, vch) && vch.size() > 0)
    {
        CDataStream s = CDataStream(vch, SER_NETWORK, PROTOCOL_VERSION);

        CBaseChainObject *pobj;
        while (!s.empty() && (pobj = RehydrateChainObject(s)))
        {
            vRet.push_back(pobj);
        }
    }
    return vRet;
}

CNodeData::CNodeData(UniValue &obj)
{
    networkAddress = uni_get_str(find_value(obj, "networkaddress"));
    paymentAddress = uni_get_str(find_value(obj, "paymentaddress"));
}

UniValue CNodeData::ToUniValue() const
{
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("networkaddress", networkAddress));
    obj.push_back(Pair("paymentaddress", paymentAddress));
    return obj;
}

CPBaaSChainDefinition::CPBaaSChainDefinition(const UniValue &obj)
{
    nVersion = PBAAS_VERSION;
    name = uni_get_str(find_value(obj, "name"));
    address = uni_get_str(find_value(obj, "address"));
    premine = uni_get_int64(find_value(obj, "premine"));
    conversion = uni_get_int64(find_value(obj, "conversion"));
    launchFee = uni_get_int64(find_value(obj, "launchfee"));
    startBlock = uni_get_int(find_value(obj, "startblock"));
    endBlock = uni_get_int(find_value(obj, "endblock"));

    auto vEras = uni_getValues(find_value(obj, "eras"));
    if (vEras.size() > ASSETCHAINS_MAX_ERAS)
    {
        vEras.resize(ASSETCHAINS_MAX_ERAS);
    }
    eras = !vEras.size() ? 1 : vEras.size();

    for (auto era : vEras)
    {
        rewards.push_back(uni_get_int64(find_value(era, "reward")));
        rewardsDecay.push_back(uni_get_int64(find_value(era, "decay")));
        halving.push_back(uni_get_int64(find_value(era, "halving")));
        eraEnd.push_back(uni_get_int64(find_value(era, "eraend")));
        eraOptions.push_back(uni_get_int64(find_value(era, "eraoptions")));
    }

    billingPeriod = uni_get_int(find_value(obj, "billingperiod"));
    notarizationReward = uni_get_int64(find_value(obj, "notarizationreward"));

    auto nodeVec = uni_getValues(find_value(obj, "nodes"));
    for (auto node : nodeVec)
    {
        nodes.push_back(CNodeData(node));
    }
}

CPBaaSChainDefinition::CPBaaSChainDefinition(const CTransaction &tx, bool validate)
{
    bool definitionFound = false;
    nVersion = PBAAS_VERSION_INVALID;
    for (auto out : tx.vout)
    {
        uint32_t ecode;
        if (out.scriptPubKey.IsPayToCryptoCondition(&ecode))
        {
            if (ecode == EVAL_PBAASDEFINITION)
            {
                if (definitionFound)
                {
                    nVersion = PBAAS_VERSION_INVALID;
                }
                else
                {
                    COptCCParams p;
                    definitionFound = true;
                    if (!IsPayToCryptoCondition(out.scriptPubKey, p, *this))
                    {
                        nVersion = PBAAS_VERSION_INVALID;
                    }
                }
            }
        }
    }

    if (validate)
    {
        
    }
}

uint160 CPBaaSChainDefinition::GetChainID(std::string name)
{
    const char *chainName = name.c_str();
    uint256 chainHash = Hash(chainName, chainName + strlen(chainName));
    return Hash160(chainHash.begin(), chainHash.end());
}

uint160 CPBaaSChainDefinition::GetConditionID(int32_t condition)
{
    return CCrossChainRPCData::GetConditionID(name, condition);
}

UniValue CPBaaSChainDefinition::ToUniValue() const
{
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("version", (int64_t)nVersion));
    obj.push_back(Pair("name", name));
    obj.push_back(Pair("address", address));
    obj.push_back(Pair("premine", (int64_t)premine));
    obj.push_back(Pair("conversion", (int64_t)conversion));
    obj.push_back(Pair("launchfee", (int64_t)launchFee));
    obj.push_back(Pair("conversionpercent", (double)conversion / 100000000));
    obj.push_back(Pair("launchfeepercent", ((double)launchFee / 100000000) * 100));
    obj.push_back(Pair("startblock", (int32_t)startBlock));
    obj.push_back(Pair("endblock", (int32_t)endBlock));

    UniValue eraArr(UniValue::VARR);
    for (int i = 0; i < eras; i++)
    {
        UniValue era(UniValue::VOBJ);
        era.push_back(Pair("reward", rewards.size() > i ? rewards[i] : (int64_t)0));
        era.push_back(Pair("decay", rewardsDecay.size() > i ? rewardsDecay[i] : (int64_t)0));
        era.push_back(Pair("halving", halving.size() > i ? (int32_t)halving[i] : (int32_t)0));
        era.push_back(Pair("eraend", eraEnd.size() > i ? (int32_t)eraEnd[i] : (int32_t)0));
        era.push_back(Pair("eraoptions", eraOptions.size() > i ? (int32_t)eraOptions[i] : (int32_t)0));
        eraArr.push_back(era);
    }
    obj.push_back(Pair("eras", eraArr));

    obj.push_back(Pair("billingperiod", billingPeriod));
    obj.push_back(Pair("notarizationreward", notarizationReward));

    UniValue nodeArr(UniValue::VARR);
    for (auto node : nodes)
    {
        nodeArr.push_back(node.ToUniValue());
    }
    obj.push_back(Pair("nodes", nodeArr));

    return obj;
}

#define _ASSETCHAINS_TIMELOCKOFF 0xffffffffffffffff
extern uint64_t ASSETCHAINS_TIMELOCKGTE;
extern int64_t ASSETCHAINS_SUPPLY, ASSETCHAINS_REWARD[3], ASSETCHAINS_DECAY[3], ASSETCHAINS_HALVING[3], ASSETCHAINS_ENDSUBSIDY[3];
extern int32_t PBAAS_STARTBLOCK, PBAAS_ENDBLOCK, ASSETCHAINS_LWMAPOS, ASSETCHAINS_TIMEUNLOCKFROM, ASSETCHAINS_TIMEUNLOCKTO;
extern uint32_t ASSETCHAINS_ALGO, ASSETCHAINS_VERUSHASH, ASSETCHAINS_LASTERA;
extern std::string VERUS_CHAINNAME;

// adds the chain definition for this chain and nodes as well
// this also sets up the notarization chain, if there is one
bool SetThisChain(UniValue &chainDefinition)
{
    ConnectedChains.ThisChain() = CPBaaSChainDefinition(chainDefinition);

    if (ConnectedChains.ThisChain().IsValid())
    {
        // set all command line parameters into mapArgs from chain definition
        vector<string> nodeStrs;
        for (auto node : ConnectedChains.ThisChain().nodes)
        {
            nodeStrs.push_back(node.networkAddress);
        }
        if (nodeStrs.size())
        {
            mapMultiArgs["-seednode"] = nodeStrs;
        }

        ASSETCHAINS_SUPPLY = ConnectedChains.ThisChain().premine;
        ASSETCHAINS_ALGO = ASSETCHAINS_VERUSHASH;
        ASSETCHAINS_LWMAPOS = 50;

        ASSETCHAINS_TIMELOCKGTE = _ASSETCHAINS_TIMELOCKOFF;
        ASSETCHAINS_TIMEUNLOCKFROM = 0;
        ASSETCHAINS_TIMEUNLOCKTO = 0;

        auto numEras = ConnectedChains.ThisChain().eras;
        ASSETCHAINS_LASTERA = numEras - 1;
        mapArgs["-ac_eras"] = to_string(numEras);

        mapArgs["-ac_end"] = "";
        mapArgs["-ac_reward"] = "";
        mapArgs["-ac_halving"] = "";
        mapArgs["-ac_decay"] = "";

        for (int j = 0; j < ASSETCHAINS_MAX_ERAS; j++)
        {
            if (j > ASSETCHAINS_LASTERA)
            {
                ASSETCHAINS_REWARD[j] = ASSETCHAINS_REWARD[j-1];
                ASSETCHAINS_DECAY[j] = ASSETCHAINS_DECAY[j-1];
                ASSETCHAINS_HALVING[j] = ASSETCHAINS_HALVING[j-1];
                ASSETCHAINS_ENDSUBSIDY[j] = 0;
            }
            else
            {
                ASSETCHAINS_REWARD[j] = ConnectedChains.ThisChain().rewards[j];
                ASSETCHAINS_DECAY[j] = ConnectedChains.ThisChain().rewardsDecay[j];
                ASSETCHAINS_HALVING[j] = ConnectedChains.ThisChain().halving[j];
                ASSETCHAINS_ENDSUBSIDY[j] = ConnectedChains.ThisChain().eraEnd[j];
                if (j == 0)
                {
                    mapArgs["-ac_reward"] = to_string(ASSETCHAINS_REWARD[j]);
                    mapArgs["-ac_decay"] = to_string(ASSETCHAINS_DECAY[j]);
                    mapArgs["-ac_halving"] = to_string(ASSETCHAINS_HALVING[j]);
                    mapArgs["-ac_end"] = to_string(ASSETCHAINS_ENDSUBSIDY[j]);
                }
                else
                {
                    mapArgs["-ac_reward"] += "," + to_string(ASSETCHAINS_REWARD[j]);
                    mapArgs["-ac_decay"] += "," + to_string(ASSETCHAINS_DECAY[j]);
                    mapArgs["-ac_halving"] += "," + to_string(ASSETCHAINS_HALVING[j]);
                    mapArgs["-ac_end"] += "," + to_string(ASSETCHAINS_ENDSUBSIDY[j]);
                }
            }
        }

        PBAAS_STARTBLOCK = ConnectedChains.ThisChain().startBlock;
        mapArgs["-startblock"] = to_string(PBAAS_STARTBLOCK);
        PBAAS_ENDBLOCK = ConnectedChains.ThisChain().endBlock;
        mapArgs["-endblock"] = to_string(PBAAS_ENDBLOCK);

        return true;
    }
    else
    {
        return false;
    }
}

// ensures that the chain definition is valid and that there are no other definitions of the same name
// that have been confirmed.
bool ValidateChainDefinition(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{
    // the chain definition output can be spent when the chain is at the end of its life and only then
    // TODO
    return false;
}

// ensures that the chain definition is valid and that there are no other definitions of the same name
// that have been confirmed.
bool CheckChainDefinitionOutput(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{
    // checked before a chain definition output script is accepted as a valid transaction

    // basics - we need a chain definition transaction to kick off a PBaaS chain. it must have:
    // 1) valid chain definition output with parameters in proper ranges and no duplicate name
    // 2) notarization output with conformant values
    // 3) finalization output
    // 3) notarization funding
    //

    // get the source transaction
    uint256 blkHash;
    CTransaction thisTx;
    if (!myGetTransaction(tx.vin[nIn].prevout.hash, thisTx, blkHash))
    {
        LogPrintf("failed to retrieve transaction %s\n", tx.vin[nIn].prevout.hash.GetHex().c_str());
        return false;
    }

    CPBaaSChainDefinition chainDef(thisTx, true);
    CPBaaSNotarization notarization(thisTx, true);
    CNotarizationFinalization finalization(thisTx, true);

    if (!chainDef.IsValid() || !notarization.IsValid() || finalization.IsValid())
    {
        LogPrintf("transaction specified, %s, must have valid chain definition, notarization, and finaization outputs\n", tx.vin[nIn].prevout.hash.GetHex().c_str());
        return false;
    }

    CPBaaSChainDefinition prior;
    // this ensures that there is no other definition of the same name already on the blockchain
    if (!GetChainDefinition(chainDef.name, prior))
    {
        LogPrintf("PBaaS chain with the name %s already exists\n", chainDef.name.c_str());
        return false;
    }

    return true;
}

bool CConnectedChains::RemoveMergedBlock(uint160 chainID)
{
    LOCK(cs_mergemining);
    auto chainIt = mergeMinedChains.find(chainID);
    if (chainIt != mergeMinedChains.end())
    {
        arith_uint256 target;
        target.SetCompact(chainIt->second.block.nBits);
        for (auto removeRange = mergeMinedTargets.equal_range(target); removeRange.first != removeRange.second; removeRange.first++)
        {
            // make sure we don't just match by target
            if (removeRange.first->second->GetChainID() == chainID)
            {
                mergeMinedTargets.erase(removeRange.first);
                break;
            }
        }
        mergeMinedChains.erase(chainID);
        dirtyCounter++;

        // if we get to 0, give the thread a kick to stop waiting for mining
        if (!mergeMinedChains.size())
        {
            sem_submitthread.post();
        }
    }
}

// remove merge mined chains added and not updated since a specific time
uint32_t CConnectedChains::PruneOldChains(uint32_t pruneBefore)
{
    vector<uint160> toRemove;

    LOCK(cs_mergemining);
    for (auto blkData : mergeMinedChains)
    {
        if (blkData.second.block.nTime < pruneBefore)
        {
            toRemove.push_back(blkData.first);
        }
    }

    for (auto id : toRemove)
    {
        RemoveMergedBlock(id);
    }
}

// adds or updates merge mined blocks
// returns false if failed to add
bool CConnectedChains::AddMergedBlock(CPBaaSMergeMinedChainData &blkData)
{
    bool blockSet = false;
    int idx = -1;
    // determine if we should replace one or add to the merge mine vector
    {
        LOCK(cs_mergemining);

        uint160 cID = blkData.GetChainID();
        auto it = mergeMinedChains.find(cID);
        if (it != mergeMinedChains.end())
        {
            // replace data
            it->second = blkData;
        }
        else
        {
            arith_uint256 target;
            target.SetCompact(blkData.block.nBits);
            mergeMinedTargets.insert(make_pair(target, &(mergeMinedChains.insert(make_pair(cID, blkData)).first->second)));
        }
        dirtyCounter++;
    }
    return true;
}

bool CConnectedChains::GetChainInfo(uint160 chainID, CRPCChainData &rpcChainData)
{
    {
        LOCK(cs_mergemining);
        auto chainIt = mergeMinedChains.find(chainID);
        if (chainIt != mergeMinedChains.end())
        {
            rpcChainData = (CRPCChainData)chainIt->second;
            return true;
        }
        return false;
    }
}

// this returns a pointer to the data without copy and assumes the lock is held
CPBaaSMergeMinedChainData *CConnectedChains::GetChainInfo(uint160 chainID)
{
    {
        auto chainIt = mergeMinedChains.find(chainID);
        if (chainIt != mergeMinedChains.end())
        {
            return &chainIt->second;
        }
        return NULL;
    }
}

// get the latest block header and submit one block at a time, returning after there are no more
// matching blocks to be found
vector<pair<string, UniValue>> CConnectedChains::SubmitQualifiedBlocks()
{
    std::set<uint160> inHeader;
    bool submissionFound;
    CPBaaSMergeMinedChainData chainData;
    vector<pair<string, UniValue>>  results;

    CBlockHeader bh;
    arith_uint256 lastHash;
    CPBaaSBlockHeader pbh;

    {
        // work through the current latest data once and return
        LOCK(cs_mergemining);
        bh = latestBlockHeader;
        lastHash = latestHash;
    }

    // loop through the existing PBaaS chain ids in the header and add them to ax set
    for (uint32_t i = 0; bh.GetPBaaSHeader(pbh, i); i++)
    {
        inHeader.insert(pbh.chainID);
    }

    {
        do
        {
            submissionFound = false;
            {
                LOCK(cs_mergemining);
                for (auto chainIt = mergeMinedTargets.lower_bound(lastHash); !submissionFound && chainIt != mergeMinedTargets.end(); chainIt++)
                {
                    uint160 chainID = chainIt->second->GetChainID();
                    if (inHeader.count(chainID))
                    {
                        // first, check that the winning header matches the block that is there
                        CPBaaSPreHeader preHeader(chainIt->second->block);
                        preHeader.SetBlockData(bh);

                        // check if the block header matches the block's specific data, only then can we create a submission from this block
                        if (bh.CheckNonCanonicalData())
                        {
                            // save block as is, remove the block from merged headers, replace header, and submit
                            chainData = *chainIt->second;

                            *(CBlockHeader *)&chainData.block = bh;

                            // once it is going to be submitted, remove it until it is added again
                            RemoveMergedBlock(chainID);

                            submissionFound = true;
                        }
                    }
                }
            }
            if (submissionFound)
            {
                // submit one block and loop again. this approach allows multiple threads
                // to collectively empty the submission queue, mitigating the impact of
                // any one stalled daemon
                UniValue submitParams(UniValue::VARR);
                submitParams.push_back(EncodeHexBlk(chainData.block));
                UniValue result;
                try
                {
                    result = RPCCall("submitblock", submitParams, chainData.rpcUserPass, chainData.rpcPort, chainData.rpcHost);
                }
                catch (exception e)
                {
                    result = UniValue(e.what());
                }
                results.push_back(make_pair(chainData.chainDefinition.name, result));
                if (result.isStr())
                {
                    printf("Submission results for %s chain: %s\n", chainData.chainDefinition.name.c_str(), result.get_str().c_str());
                }
                else
                {
                    printf("Successfully submitted block to %s chain\n", chainData.chainDefinition.name.c_str());

                }
            }
        } while (submissionFound);
    }
    return results;
}

// add all merge mined chain PBaaS headers into the blockheader and return the easiest nBits target in the header
uint32_t CConnectedChains::CombineBlocks(CBlockHeader &bh)
{
    vector<uint160> inHeader;
    vector<UniValue> toCombine;
    arith_uint256 blkHash = UintToArith256(bh.GetHash());
    arith_uint256 target(0);
    
    CPBaaSBlockHeader pbh;

    {
        LOCK(cs_mergemining);

        CPBaaSSolutionDescriptor descr = CVerusSolutionVector::solutionTools.GetDescriptor(bh.nSolution);

        for (uint32_t i = 0; i < descr.numPBaaSHeaders; i++)
        {
            if (bh.GetPBaaSHeader(pbh, i))
            {
                inHeader.push_back(pbh.chainID);
            }
        }

        // loop through the existing PBaaS chain ids in the header
        // remove any not either this Chain ID or in our local collection and then add all that are present
        for (uint32_t i = 0; i < inHeader.size(); i++)
        {
            auto it = mergeMinedChains.find(inHeader[i]);
            if (inHeader[i] != ASSETCHAINS_CHAINID && (it == mergeMinedChains.end()))
            {
                bh.DeletePBaaSHeader(i);
            }
            else
            {
                arith_uint256 t;
                t.SetCompact(it->second.block.nBits);
                if (t > target)
                {
                    target = t;
                }
            }
        }

        for (auto chain : mergeMinedChains)
        {
            // get the native PBaaS header for each chain and put it into the
            // header we are given
            uint160 cid = chain.second.GetChainID();
            if (chain.second.block.GetPBaaSHeader(pbh, cid))
            {
                if (!bh.AddUpdatePBaaSHeader(pbh))
                {
                    LogPrintf("Failure to add PBaaS block header for %s chain\n", chain.second.chainDefinition.name.c_str());
                    break;
                }
                else
                {
                    arith_uint256 t;
                    t.SetCompact(chain.second.block.nBits);
                    if (t > target)
                    {
                        target = t;
                    }
                }
            }
        }
    }
    return target.GetCompact();
}

bool CConnectedChains::IsVerusPBaaSAvailable()
{
    return notaryChainVersion > "0.6";
}

extern string PBAAS_HOST, PBAAS_USERPASS;
extern int32_t PBAAS_PORT;
bool CConnectedChains::CheckVerusPBaaSAvailable(UniValue &chainInfoUni, UniValue &chainDefUni)
{
    if (chainInfoUni.isObject() && chainDefUni.isObject())
    {
        UniValue uniVer = find_value(chainInfoUni, "VRSCversion");
        if (uniVer.isStr())
        {
            notaryChainVersion = uni_get_str(uniVer);
            notaryChainHeight = uni_get_int(find_value(chainInfoUni, "blocks"));
            CPBaaSChainDefinition chainDef(chainDefUni);
            notaryChain = CRPCChainData(chainDef, PBAAS_HOST, PBAAS_PORT, PBAAS_USERPASS);
        }
    }
    return IsVerusPBaaSAvailable();
}

bool CConnectedChains::CheckVerusPBaaSAvailable()
{
    if (IsVerusActive())
    {
        notaryChainVersion = "";
    }
    else
    {
        // if this is a PBaaS chain, poll for presence of Verus / root chain and current Verus block and version number
        UniValue chainInfo, chainDef;
        try
        {
            UniValue params(UniValue::VARR);
            chainInfo = find_value(RPCCallRoot("getinfo", params), "result");
            if (!chainInfo.isNull())
            {
                params.push_back(VERUS_CHAINNAME);
                chainDef = find_value(RPCCallRoot("getchaindefinition", params), "result");

                if (!chainDef.isNull() && CheckVerusPBaaSAvailable(chainInfo, chainDef))
                {
                    return true;
                }
            }
        } catch (exception e)
        {
        }
    }
    notaryChainVersion = "";
    return false;
}

void CConnectedChains::SubmissionThread()
{
    try
    {
        arith_uint256 lastHash;
        
        {
            LOCK(cs_mergemining);
            lastHash = latestHash;
        }

        // wait for something to check on, then submit blocks that should be submitted
        while (true)
        {
            if (IsVerusActive())
            {
                // blocks get discarded after no refresh for 5 minutes by default
                ConnectedChains.PruneOldChains(GetAdjustedTime() - 300);
                if (mergeMinedChains.size() > 0)
                {
                    sem_submitthread.wait();
                    // wait for a new block header win
                    {
                        bool submit = false;
                        {
                            LOCK(cs_mergemining);
                            if (lastHash != latestHash)
                            {
                                submit = true;
                            }
                        }
                        if (submit)
                        {
                            SubmitQualifiedBlocks();
                        }
                    }
                }
                else
                {
                    MilliSleep(500);
                }
            }
            else
            {
                UniValue result;

                // if this is a PBaaS chain, poll for presence of Verus / root chain and current Verus block and version number
                CheckVerusPBaaSAvailable();
                sleep(3);
            }

            boost::this_thread::interruption_point();
        }
    }
    catch (const boost::thread_interrupted&)
    {
        LogPrintf("Verus merge mining thread terminated\n");
    }
}

void CConnectedChains::SubmissionThreadStub()
{
    ConnectedChains.SubmissionThread();
}

bool IsChainDefinitionInput(const CScript &scriptSig)
{
    uint32_t ecode;
    return scriptSig.IsPayToCryptoCondition(&ecode) && ecode == EVAL_PBAASDEFINITION;
}