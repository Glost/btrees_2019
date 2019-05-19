/// \file
/// \brief     B-tree experiments for calculating coefficients for the algorithm of selecting the index structure.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      18.05.2019 -- 18.05.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include <malloc.h>
#include <ctime>
#include <iostream>

#include "btree.h"

using namespace btree;

const int MIN_INSERTS_COUNT = 500;
const int MAX_INSERTS_COUNT = 1000;
const int MIN_TREE_ORDER = 100;
const int MAX_TREE_ORDER = 1000;
const int TREE_ORDER_STEP = 50;

struct IntComparator : public BaseBTree::IComparator
{

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        return *((int*) lhv) < *((int*) rhv);
    }

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        return *((int*) lhv) == *((int*) rhv);
    }

};

typedef struct IntComparator IntComparator;

IntComparator intComparator;

void doExpOnIncreasingKeys(std::ostream& csv, UShort treeOrder);

void doExpOnDecreasingKeys(std::ostream& csv, UShort treeOrder);

void doExpOnRandomKeys(std::ostream& csv, UShort treeOrder);

void* operator new(size_t size);

void operator delete(void* p) noexcept;

void* operator new[](size_t size);

void operator delete[](void* p) noexcept;

size_t currentUsedMemory = 0;

size_t maxUsedMemory = 0;

int main()
{
    std::ofstream csv("btrees_coef_exps.csv");

    if (!csv.is_open())
        throw std::runtime_error("Can't open CSV file for writing");

    csv << "Tree Order;Keys Order;Insertions Count;Deletions Count"
            << ";B-tree Time;B-tree Max Used Memory"
            << ";B+-tree Time;B+-tree Max Used Memory"
            << ";B*-tree Time;B*-tree Max Used Memory"
            << ";B*+-tree Time;B*+-tree Max Used Memory"
            << std::endl;

    for (UShort treeOrder = MIN_TREE_ORDER; treeOrder <= MAX_TREE_ORDER; treeOrder += TREE_ORDER_STEP)
    {
        doExpOnIncreasingKeys(csv, treeOrder);
        doExpOnDecreasingKeys(csv, treeOrder);
        doExpOnRandomKeys(csv, treeOrder);

        std::cout << "Tree order " << treeOrder << " handled" << std::endl;
    }

    csv.close();

    return 0;
}

void doExpOnIncreasingKeys(std::ostream& csv, UShort treeOrder)
{
    for (int i = MIN_INSERTS_COUNT; i <= MAX_INSERTS_COUNT; ++i)
    {
        FileBaseBTree* bTree = new FileBaseBTree(BaseBTree::TreeType::B_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BTree.btree"));
        FileBaseBTree* bPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BPlusTree.btree"));
        FileBaseBTree* bStarTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarTree.btree"));
        FileBaseBTree* bStarPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarPlusTree.btree"));

        maxUsedMemory = 0;
        clock_t start = std::clock();

        for (int j = 0; j < i; ++j)
            bTree->insert((Byte*) &j);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bTree->remove((Byte*) &j);

        clock_t end = std::clock();
        clock_t bTreeTime = end - start;
        size_t bTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bPlusTree->insert((Byte*) &j);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bPlusTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bPlusTreeTime = end - start;
        size_t bPlusTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bStarTree->insert((Byte*) &j);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bStarTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bStarTreeTime = end - start;
        size_t bStarTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bStarPlusTree->insert((Byte*) &j);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bStarPlusTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bStarPlusTreeTime = end - start;
        size_t bStarPlusTreeMaxUsedMemory = maxUsedMemory;

        csv << treeOrder << ";Increasing;" << i << ";" << MAX_INSERTS_COUNT - i
                << ";" << bTreeTime << ";" << bTreeMaxUsedMemory
                << ";" << bPlusTreeTime << ";" << bPlusTreeMaxUsedMemory
                << ";" << bStarTreeTime << ";" << bStarTreeMaxUsedMemory
                << ";" << bStarPlusTreeTime << ";" << bStarPlusTreeMaxUsedMemory
                << std::endl;

        delete bTree;
        delete bPlusTree;
        delete bStarTree;
        delete bStarPlusTree;
    }
}

void doExpOnDecreasingKeys(std::ostream& csv, UShort treeOrder)
{
    for (int i = MIN_INSERTS_COUNT; i <= MAX_INSERTS_COUNT; ++i)
    {
        FileBaseBTree* bTree = new FileBaseBTree(BaseBTree::TreeType::B_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BTree.btree"));
        FileBaseBTree* bPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BPlusTree.btree"));
        FileBaseBTree* bStarTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarTree.btree"));
        FileBaseBTree* bStarPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarPlusTree.btree"));

        maxUsedMemory = 0;
        clock_t start = std::clock();

        for (int j = i; j > 0; --j)
            bTree->insert((Byte*) &j);

        for (int j = i; j > i - (MAX_INSERTS_COUNT - i); --j)
            bTree->remove((Byte*) &j);

        clock_t end = std::clock();
        clock_t bTreeTime = end - start;
        size_t bTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = i; j > 0; --j)
            bPlusTree->insert((Byte*) &j);

        for (int j = i; j > i - (MAX_INSERTS_COUNT - i); --j)
            bPlusTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bPlusTreeTime = end - start;
        size_t bPlusTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = i; j > 0; --j)
            bStarTree->insert((Byte*) &j);

        for (int j = i; j > i - (MAX_INSERTS_COUNT - i); --j)
            bStarTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bStarTreeTime = end - start;
        size_t bStarTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = i; j > 0; --j)
            bStarPlusTree->insert((Byte*) &j);

        for (int j = i; j > i - (MAX_INSERTS_COUNT - i); --j)
            bStarPlusTree->remove((Byte*) &j);

        end = std::clock();
        clock_t bStarPlusTreeTime = end - start;
        size_t bStarPlusTreeMaxUsedMemory = maxUsedMemory;

        csv << treeOrder  << ";Decreasing;" << i << ";" << MAX_INSERTS_COUNT - i
                << ";" << bTreeTime << ";" << bTreeMaxUsedMemory
                << ";" << bPlusTreeTime << ";" << bPlusTreeMaxUsedMemory
                << ";" << bStarTreeTime << ";" << bStarTreeMaxUsedMemory
                << ";" << bStarPlusTreeTime << ";" << bStarPlusTreeMaxUsedMemory
                << std::endl;

        delete bTree;
        delete bPlusTree;
        delete bStarTree;
        delete bStarPlusTree;
    }
}

void doExpOnRandomKeys(std::ostream& csv, UShort treeOrder)
{
    srand(time(NULL));

    for (int i = MIN_INSERTS_COUNT; i <= MAX_INSERTS_COUNT; ++i)
    {
        int* randomKeysArr = new int[i];
        size_t randomKeysArrSize = _msize(randomKeysArr);
        currentUsedMemory -= randomKeysArrSize;

        for (int j = 0; j < i; ++j)
            randomKeysArr[j] = rand();

        FileBaseBTree* bTree = new FileBaseBTree(BaseBTree::TreeType::B_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BTree.btree"));
        FileBaseBTree* bPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BPlusTree.btree"));
        FileBaseBTree* bStarTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarTree.btree"));
        FileBaseBTree* bStarPlusTree = new FileBaseBTree(BaseBTree::TreeType::B_STAR_PLUS_TREE, treeOrder, sizeof(int),
                &intComparator, std::string("BStarPlusTree.btree"));

        maxUsedMemory = 0;
        clock_t start = std::clock();

        for (int j = 0; j < i; ++j)
            bTree->insert((Byte*) &randomKeysArr[j]);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bTree->remove((Byte*) &randomKeysArr[j]);

        clock_t end = std::clock();
        clock_t bTreeTime = end - start;
        size_t bTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bPlusTree->insert((Byte*) &randomKeysArr[j]);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bPlusTree->remove((Byte*) &randomKeysArr[j]);

        end = std::clock();
        clock_t bPlusTreeTime = end - start;
        size_t bPlusTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bStarTree->insert((Byte*) &randomKeysArr[j]);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bStarTree->remove((Byte*) &randomKeysArr[j]);

        end = std::clock();
        clock_t bStarTreeTime = end - start;
        size_t bStarTreeMaxUsedMemory = maxUsedMemory;

        maxUsedMemory = 0;
        start = std::clock();

        for (int j = 0; j < i; ++j)
            bStarPlusTree->insert((Byte*) &randomKeysArr[j]);

        for (int j = 0; j < MAX_INSERTS_COUNT - i; ++j)
            bStarPlusTree->remove((Byte*) &randomKeysArr[j]);

        end = std::clock();
        clock_t bStarPlusTreeTime = end - start;
        size_t bStarPlusTreeMaxUsedMemory = maxUsedMemory;

        csv << treeOrder  << ";Random;" << i << ";" << MAX_INSERTS_COUNT - i
                << ";" << bTreeTime << ";" << bTreeMaxUsedMemory
                << ";" << bPlusTreeTime << ";" << bPlusTreeMaxUsedMemory
                << ";" << bStarTreeTime << ";" << bStarTreeMaxUsedMemory
                << ";" << bStarPlusTreeTime << ";" << bStarPlusTreeMaxUsedMemory
                << std::endl;

        delete[] randomKeysArr;
        currentUsedMemory += randomKeysArrSize;

        delete bTree;
        delete bPlusTree;
        delete bStarTree;
        delete bStarPlusTree;
    }
}

void* operator new(size_t size)
{
    void* p = malloc(size);
    currentUsedMemory += _msize(p);
    if (currentUsedMemory > maxUsedMemory)
        maxUsedMemory = currentUsedMemory;
    return p;
}

void operator delete(void* p) noexcept
{
    currentUsedMemory -= _msize(p);
    free(p);
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* p) noexcept
{
    ::operator delete(p);
}
