/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 22.12.2018
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_BTREE_C_H
#define BTREES_BTREE_C_H

#include "btree.h"

using namespace btree;

struct ByteComparator : public BaseBTree::IComparator {

    UInt firstPartBytes = 0;

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz && i < firstPartBytes; ++i)
        {
            if (lhv[i] < rhv[i])
                return true;
            if (lhv[i] > rhv[i])
                return false;
        }

        return false;
    }

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz && i < firstPartBytes; ++i)
            if (lhv[i] != rhv[i])
                return false;

        return true;
    }

}; // struct ByteComparator

typedef struct ByteComparator ByteComparator;

#ifdef __cplusplus
extern "C" {
#endif

static ByteComparator byteComparator;

static void create(FileBaseBTree** pTree, BaseBTree::TreeType treeType,
        UShort order, UShort keySize, const char* treeFileName);

static void createBTree(FileBaseBTree** pTree, UShort order, UShort keySize, const char* treeFileName);

static void open(FileBaseBTree** pTree, BaseBTree::TreeType treeType, const char* treeFileName);

static void close(FileBaseBTree** pTree);

static void insert(FileBaseBTree** pTree, const Byte* k) { (*pTree)->insert(k); }

static Byte* search(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->search(k); }

static int searchAll(FileBaseBTree** pTree, const Byte* k, Byte*** keysPointer);

#ifdef BTREE_WITH_DELETION

static bool removeKey(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->remove(k); }

static int removeAll(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->removeAll(k); }

#endif

#ifdef __cplusplus
}; // extern "C"
#endif

#include "btree_c.hpp"

#endif //BTREES_BTREE_C_H
