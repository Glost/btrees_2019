/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 04.05.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_BTREE_C_H
#define BTREES_BTREE_C_H

#include "btree.h" // The B-tree modifications C++ library.

using namespace btree;

#define INTEGER_SIZE 4
#define FLOAT_SIZE 8
#define TEXT_SIZE 256
#define NULL_SIZE 1

/**
 * The simple byte comparator for the tree.
 */
struct ByteComparator : public BaseBTree::IComparator {

    /**
     * The bytes count of the first part (the primary key value part) of the tree keys.
     */
    UInt firstPartBytes = 0;

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override;

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override;
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

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override;

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override { return true; }
}; // struct SearchAllByteComparator

typedef struct SearchAllByteComparator SearchAllByteComparator;

/**
 * The byte printer for writing the key into the GraphViz file byte-by-byte.
 */
struct BytePrinter : public BaseBTree::IKeyPrinter {

    UInt firstPartBytes = TEXT_SIZE;

    virtual std::string print(const Byte* key, UInt sz) override;
}; // struct BytePrinter

typedef struct BytePrinter BytePrinter;

/**
 * The integer byte printer for writing the integer key into the GraphViz file.
 */
struct IntBytePrinter : public BaseBTree::IKeyPrinter {

    UInt firstPartBytes = INTEGER_SIZE;

    virtual std::string print(const Byte* key, UInt sz) override
            { return std::to_string(*((int*) key)) + std::string("; ") +
                      std::to_string(*((long long*) &key[firstPartBytes])); }
}; // struct IntBytePrinter

typedef struct IntBytePrinter IntBytePrinter;

/**
 * The float byte printer for writing the float key into the GraphViz file.
 */
struct FloatBytePrinter : public BaseBTree::IKeyPrinter {

    UInt firstPartBytes = FLOAT_SIZE;

    virtual std::string print(const Byte* key, UInt sz) override
            { return std::to_string(*((double*) key)) + std::string("; ") +
                      std::to_string(*((long long*) &key[firstPartBytes])); }
}; // struct FloatBytePrinter

typedef struct FloatBytePrinter FloatBytePrinter;

/**
 * The null byte printer for writing the null key into the GraphViz file.
 */
struct NullBytePrinter : public BaseBTree::IKeyPrinter {

    UInt firstPartBytes = NULL_SIZE;

    virtual std::string print(const Byte* key, UInt sz) override { return std::string("NULL"); }
}; // struct NullBytePrinter

typedef struct NullBytePrinter NullBytePrinter;

#ifdef __cplusplus
extern "C" {
#endif

static ByteComparator byteComparator;

static SearchAllByteComparator searchAllByteComparator;

static BytePrinter bytePrinter;

static IntBytePrinter intBytePrinter;

static FloatBytePrinter floatBytePrinter;

static NullBytePrinter nullBytePrinter;

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
 * @param tree The pointer to the given tree.
 */
static void setByteComparator(FileBaseBTree* tree) { tree->getTree()->setComparator(&byteComparator); }

/**
 * Sets the byte comparator which searches for all the tree keys to the given tree.
 *
 * @param tree The pointer to the given tree.
 */
static void setSearchAllByteComparator(FileBaseBTree* tree)
        { tree->getTree()->setComparator(&searchAllByteComparator); }

/**
 * Inserts the key into the tree.
 *
 * @param tree The pointer to the tree.
 * @param k The inserted key.
 */
static void insert(FileBaseBTree* tree, const Byte* k) { tree->insert(k); }

/**
 * Searches for the key in the tree.
 *
 * @param tree The pointer to the tree.
 * @param k The searched key.
 * @return The found key.
 */
static Byte* search(FileBaseBTree* tree, const Byte* k) { return tree->search(k); }

/**
 * Searches for all the given keys in the tree.
 *
 * @param tree The pointer to the tree.
 * @param k The searched key.
 * @param keysPointer The pointers to the found keys array.
 * @return The found keys count.
 */
static int searchAll(FileBaseBTree* tree, const Byte* k, Byte*** keysPointer);

#ifdef BTREE_WITH_DELETION

/**
 * Removes the key from the tree.
 *
 * @param tree The pointer to the tree.
 * @param k The removed key.
 * @return true if the key was successfully removed, false otherwise.
 */
static bool removeKey(FileBaseBTree* tree, const Byte* k) { return tree->remove(k); }

/**
 * Removes all the given keys from the tree.
 *
 * @param tree The pointer to the tree.
 * @param k The removed key.
 * @return The removed keys count.
 */
static int removeAll(FileBaseBTree* tree, const Byte* k) { return tree->removeAll(k); }

#endif

/**
 * Visualizes the tree to the GraphViz DOT file.
 *
 * @param tree The pointer to the tree.
 * @param dotFileName The GraphViz DOT file name.
 * @return true if the DOT file is written, false if it is impossible to open the DOT file for writing.
 */
static bool visualize(FileBaseBTree* tree, const char* dotFileName);

/**
 * Gets order for the tree.
 *
 * @param tree The pointer to the tree.
 * @return The tree order.
 */
static int getOrder(FileBaseBTree* tree) { return tree->getTree()->getOrder(); }

/**
 * Sets the byte printer as the key printer to the tree.
 *
 * @param tree The pointer to the tree.
 */
static void setBytePrinter(FileBaseBTree* tree) { tree->getTree()->setKeyPrinter(&bytePrinter); }

/**
 * Sets the integer byte printer as the key printer to the tree.
 *
 * @param tree The pointer to the tree.
 */
static void setIntBytePrinter(FileBaseBTree* tree) { tree->getTree()->setKeyPrinter(&intBytePrinter); }

/**
 * Sets the float byte printer as the key printer to the tree.
 *
 * @param tree The pointer to the tree.
 */
static void setFloatBytePrinter(FileBaseBTree* tree) { tree->getTree()->setKeyPrinter(&floatBytePrinter); }

/**
 * Sets the null printer as the key printer to the tree.
 *
 * @param tree The pointer to the tree.
 */
static void setNullBytePrinter(FileBaseBTree* tree) { tree->getTree()->setKeyPrinter(&nullBytePrinter); }

#ifdef __cplusplus
}; // extern "C"
#endif

#include "btree_c.hpp"

#endif //BTREES_BTREE_C_H
