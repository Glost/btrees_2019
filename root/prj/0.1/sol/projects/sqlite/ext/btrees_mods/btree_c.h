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

struct FirstPartByteComparator : public BaseBTree::IComparator {

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

}; // struct FirstPartByteComparator

typedef struct FirstPartByteComparator FirstPartByteComparator;

struct SecondPartByteComparator : public BaseBTree::IComparator {

    UInt firstPartBytes = 0;

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = firstPartBytes; i < sz; ++i)
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
        for (UInt i = firstPartBytes; i < sz; ++i)
            if (lhv[i] != rhv[i])
                return false;

        return true;
    }

}; // struct SecondPartByteComparator

typedef struct SecondPartByteComparator SecondPartByteComparator;

#ifdef __cplusplus
extern "C" {
#endif

static FileBaseBTree* tree = nullptr;

static FirstPartByteComparator firstPartByteComparator;

static SecondPartByteComparator secondPartByteComparator;

static void create(BaseBTree::TreeType treeType, UShort order, UShort keySize, const char* treeFileName);

static void createBTree(UShort order, UShort keySize, const char* treeFileName);

static void open(BaseBTree::TreeType treeType, const char* treeFileName);

static void close();

static void setFirstPartByteComparator() { tree->getTree()->setComparator(&firstPartByteComparator); }

static void setSecondPartByteComparator() { tree->getTree()->setComparator(&secondPartByteComparator); }

static void insert(const Byte* k) { tree->insert(k); }

static Byte* search(const Byte* k) { return tree->search(k); }

static int searchAll(const Byte* k, Byte*** keysPointer);

#ifdef BTREE_WITH_DELETION

static bool removeKey(const Byte* k) { return tree->remove(k); }

static int removeAll(const Byte* k) { return tree->removeAll(k); }

#endif

#ifdef __cplusplus
}; // extern "C"
#endif

#include "btree_c.hpp"

#endif //BTREES_BTREE_C_H
