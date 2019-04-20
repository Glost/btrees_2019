/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 20.04.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_BTREE_C_H
#define BTREES_BTREE_C_H

#include "btree.h" // The B-tree modifications C++ library.

using namespace btree;

/**
 * The simple byte comparator for the tree.
 */
struct ByteComparator : public BaseBTree::IComparator {

    /**
     * The bytes count of the first part (the primary key value part) of the tree keys.
     */
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

/**
 * The byte comparator for the tree which searches for all the tree keys.
 */
struct SearchAllByteComparator : public BaseBTree::IComparator {

    /**
     * The bytes count of the first part (the primary key value part) of the tree keys.
     */
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
        return true;
    }

}; // struct SearchAllByteComparator

typedef struct SearchAllByteComparator SearchAllByteComparator;

#ifdef __cplusplus
extern "C" {
#endif

static ByteComparator byteComparator;

static SearchAllByteComparator searchAllByteComparator;

/**
 * Creates the tree of the given type (B-tree or one of its modifications).
 *
 * @param pTree The pointer for saving the created tree.
 * @param treeType The type of the created tree (B-tree or one of its modifications).
 * @param order The tree order.
 * @param keySize The tree key size.
 * @param treeFileName The tree file name.
 */
static void create(FileBaseBTree** pTree, BaseBTree::TreeType treeType,
        UShort order, UShort keySize, const char* treeFileName);

/**
 * Creates the B-tree.
 *
 * @param pTree The pointer for saving the created tree.
 * @param order The tree order.
 * @param keySize The tree key size.
 * @param treeFileName The tree file name.
 */
static void createBTree(FileBaseBTree** pTree, UShort order, UShort keySize, const char* treeFileName);

/**
 * Open the tree of the given type (B-tree or one of its modifications).
 *
 * @param pTree The pointer for saving the opened tree.
 * @param treeType The type of the opened tree (B-tree or one of its modifications).
 * @param treeFileName The tree file name.
 */
static void open(FileBaseBTree** pTree, BaseBTree::TreeType treeType, const char* treeFileName);

/**
 * Closes the tree.
 *
 * @param pTree The pointer of tree being closed.
 */
static void close(FileBaseBTree** pTree);

/**
 * Sets the simple byte comparator to the given tree.
 *
 * @param pTree The pointer to the given tree.
 */
static void setByteComparator(FileBaseBTree** pTree) { (*pTree)->getTree()->setComparator(&byteComparator); }

/**
 * Sets the byte comparator which searches for all the tree keys to the given tree.
 *
 * @param pTree The pointer to the given tree.
 */
static void setSearchAllByteComparator(FileBaseBTree** pTree)
        { (*pTree)->getTree()->setComparator(&searchAllByteComparator); }

/**
 * Inserts the key into the tree.
 *
 * @param pTree The pointer to the tree.
 * @param k The inserted key.
 */
static void insert(FileBaseBTree** pTree, const Byte* k) { (*pTree)->insert(k); }

/**
 * Searches for the key in the tree.
 *
 * @param pTree The pointer to the tree.
 * @param k The searched key.
 * @return The found key.
 */
static Byte* search(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->search(k); }

/**
 * Searches for all the given keys in the tree.
 *
 * @param pTree The pointer to the tree.
 * @param k The searched key.
 * @param keysPointer The pointers to the found keys array.
 * @return The found keys count.
 */
static int searchAll(FileBaseBTree** pTree, const Byte* k, Byte*** keysPointer);

#ifdef BTREE_WITH_DELETION

/**
 * Removes the key from the tree.
 *
 * @param pTree The pointer to the tree.
 * @param k The removed key.
 * @return true if the key was successfully removed, false otherwise.
 */
static bool removeKey(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->remove(k); }

/**
 * Removes all the given keys from the tree.
 *
 * @param pTree The pointer to the tree.
 * @param k The removed key.
 * @return The removed keys count.
 */
static int removeAll(FileBaseBTree** pTree, const Byte* k) { return (*pTree)->removeAll(k); }

#endif

#ifdef __cplusplus
}; // extern "C"
#endif

#include "btree_c.hpp"

#endif //BTREES_BTREE_C_H
