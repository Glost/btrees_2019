/// \file
/// \brief     B-tree, B+-tree, B*-tree and B*+-tree classes
/// \authors   Anton Rigin, also code by Sergey Shershakov used
/// \version   0.1.0
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREE_BTREE_H_
#define BTREE_BTREE_H_

#ifndef BTREE_WITH_DELETION
#define BTREE_WITH_DELETION

#ifndef BTREE_WITH_REUSING_FREE_PAGES
#define BTREE_WITH_REUSING_FREE_PAGES

#include <string>
#include <fstream>
#include <list>

#include "utils.h"

namespace btree {

/** \brief Base B-tree.
 *
 *  Class includes B-tree base components, with using binary writing of fixed size (in bytes).
 *  Typification will be in inheriting classes.
 *
 *  The BaseBTree::_order field defines tree's order, which will contain
 *  from <em>(_order - 1)<\em> to <em>(2* _order - 1)<\em> keys (excluding root, which
 *  contains not less than 1 key for non-empty tree).
 *  The BaseBTree::_recSize field defines size (length) of the key record in bytes. The record is handled,
 *  as untyped, i.e. as the byte array of the size \c _recSize. For typification it is necessary
 *  to inherit this class and to implement casting to the right type in the inherited class
 *
 *  All pages are numbered from 1, 0 is nonexistent page (nullptr).
 */
class BaseBTree {
public:

    enum TreeType { B_TREE, B_PLUS_TREE, B_STAR_TREE, B_STAR_PLUS_TREE };

#pragma pack(push, 1)                           
    /** \brief File header structure.
     *
     * We explicitly specify the compiler to pack the structure and to not perform any fields alignment.
     * This param is supported by VS and, according to the reference, gcc, too:
     * https://gcc.gnu.org/onlinedocs/gcc/Structure-Layout-Pragmas.html
     */
    struct Header {

        /** \brief The valid signature. */
        static const UInt VALID_SIGN = 0x19979AAA;
    public:
        Header() : order(0), recSize(0), sign(0) {}
        Header(UShort ord, UShort rs) : 
            order(ord), recSize(rs), sign(VALID_SIGN)
        {
        }
    public:
        /** \brief Checks structure for integrity, returns true if it is ok, otherwise returns false. */
        bool checkIntegrity();
    public:
        UInt sign;  // = 0x19979AAA;
        UShort order;
        UShort recSize;
    }; // struct Header
#pragma pack(pop)

    /** \brief The header structure offset. */
    static const Byte HEADER_OFS = 0;

    /** \brief The header structure size. */
    static const Byte HEADER_SIZE = sizeof(Header);
    
    /** \brief The current free page number counter (current existing pages counter) offset. */
    static const UInt PAGE_COUNTER_OFS = HEADER_SIZE;
    
    /** \brief The current free page number counter (current existing pages counter) size. */
    static const UInt PAGE_COUNTER_SZ = 4;

    /** \brief The cursor (page number) size. */
    static const UInt CURSOR_SZ = 4;

    /** \brief The root page number record offset. */
    static const UInt ROOT_PAGE_NUM_OFS = PAGE_COUNTER_OFS + PAGE_COUNTER_SZ; //HEADER_SIZE;

    /** \brief The root page number record size. */
    static const UInt ROOT_PAGE_NUM_SZ = CURSOR_SZ; // 4;

    /** \brief The first real page offset. */
    static const UInt FIRST_PAGE_OFS = ROOT_PAGE_NUM_OFS + ROOT_PAGE_NUM_SZ;//PAGE_COUNTER_OFS + PAGE_COUNTER_SZ;

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    /** \brief The offset of the free pages counter from the begin of the free pages numbers area. */
    static const UInt FREE_PAGES_COUNTER_OFS = 0;

    /** \brief The size of the free pages counter. */
    static const UInt FREE_PAGES_COUNTER_SZ = 4;

    /** \brief The offset of the first free page number from the begin of the free pages numbers area. */
    static const UInt FIRST_FREE_PAGE_NUM_OFS = FREE_PAGES_COUNTER_OFS + FREE_PAGES_COUNTER_SZ;

    /** \brief The size of the free page number. */
    static const UInt FREE_PAGE_NUM_SZ = CURSOR_SZ;

#endif

    /** \brief The node (page) information record offset. */
    static const UInt NODE_INFO_OFS = 0;

    /** \brief The node (page) information record size. */
    static const UInt NODE_INFO_SZ = 2;

    /** \brief The keys area in the node (page) record offset. */
    static const UInt KEYS_OFS = NODE_INFO_SZ;

    /** \brief The max heys number for the tree of any order. */
    static const UShort MAX_KEYS_NUM = 32767;

    /** \brief The mask for flag which defines whether node (page) is leaf or not. */
    static const UShort LEAF_NODE_MASK = 0x8000;

    /** \brief The wrapper above the raw bytes array.
     *
     *  Provides usable interface for access to the values of the page / key.
     */
    class PageWrapper {

    public:

        PageWrapper(BaseBTree* tr);

        virtual ~PageWrapper();

        /** \brief Reallocates memory for page. */
        void reallocData(UInt sz);

        /** \brief Clears bytes array. */
        void clear();

        /** \brief Sets two fields: keys in the page number \c keyNum and feature defines whether page is
         *  the leaf or not \c isLeaf.
         *  Checks keys number for the correctness. Throws an exception if the keys number is incorrect.
         */
        void setKeyNumLeaf(UShort keysNum, bool isRoot, bool isLeaf);

        /** \brief Sets keys in the page number \c keyNum.
         *  Checks keys number for the correctness. Throws an exception if the keys number is incorrect.
         */
        void setKeyNum(UShort keysNum, bool isRoot);

        /** \brief Overloaded setKeyNum(), defines page type. */
        void setKeyNum(UShort keysNum)
        {
            setKeyNum(keysNum, isRoot());   // getNodeType());
        }

        /** \brief Returns the keys in the page number. */
        UShort getKeysNum() const;

        /** \brief Sets flag which defines whether page is the leaf or not. */
        void setLeaf(bool isLeaf);

        /** \brief Returns flag which defines whether page is the leaf or not. */
        bool isLeaf() const;

        /** \brief Returns pointer to the raw data array. */
        Byte* getData() { return _data;  }

        /** \brief Returns const pointer to the raw data array. */
        const Byte* getData() const { return _data; }

        /** \brief Returns pointer to the memory area of key with number \c num.
         *
         *  Keys numbering starts from 0.
         *  If there is no such a key, returns nullptr.
         */
        Byte* getKey(UShort num);

        /** \brief Copies key value to the address \c dst from the address \c src.
         *
         *  Keys can belong to different pages, but the page size is taken from the current page.
         */
        inline void copyKey(Byte* dst, const Byte* src);

        /** \brief Copies several keys' values starting from the address \c dst, from the address \c src.
         *  Keys number is defined by the \c num.
         *
         *  Keys can belong to different pages, but the page size is taken from the current page.
         */
        inline void copyKeys(Byte* dst, const Byte* src, UShort num);

        /** \brief Copies cursors similar to copyKeys(). */
        inline void copyCursors(Byte* dst, const Byte* src, UShort num);

        /** \brief Overloaded const getKey(). */
        const Byte* getKey(UShort num) const;

        /** \brief Returns cursor of the number \c cnum.
         *
         *  Throws an exception if the \c cnum exceeds (n+1) (numbering starts from 0)
         *  where n is for keys in the page number.
         */
        UInt getCursor(UShort cnum);

        /** \brief Returns pointer to the cursor of the number \c cnum. */
        Byte* getCursorPtr(UShort cnum);

        /** \brief Sets the value \c cval for the cursor of the number \c cnum.
         *
         *  If there is not such a cursor, throws an exception.
         */
        void setCursor(UShort cnum, UInt cval);

        /** \brief Returns the offset (in the cursors area) of the cursor with number \c cnum.
         *
         *  If there is not such a cursor, returns -1.
         */
        int getCursorOfs(UShort cnum) const;

        /** \brief Returns the offset (in the keys area) of the key with number \c num.
         *
         *  If there is not such a key, returns -1.
         */
        int getKeyOfs(UShort num) const;

        /** \brief Returns the page number */
        UInt getPageNum() const { return _pageNum; }

        /** \brief Returns true if the page is root, otherwise returns false. */
        bool isRoot() const { return _tree->getRootPageNum() == getPageNum(); }

        /** \brief Sets the page as the root of the tree.
         *
         *  The \c writeFlag flag defines is it necessary to write the new root page number directly to the file.
         *  If there is no page in the file for this wrapper and \c writeFlag == true, throws an exception.
         */
        void setAsRoot(bool writeFlag = true);

        /** \brief Returns true if the page is fulfilled, otherwise returns false.  */
        bool isFull() const { return _tree->isFull(*this); };

    public:

        /** \brief Allocates the next page in the file and associates the wrapper with this page.
         *
         *  Requirements are similar to BaseBTree::allocPage().
         */
        void allocPage(UShort keysNum, bool isLeaf)
        {
            _pageNum = _tree->allocPage(*this, keysNum, isLeaf);
        }

        /** \brief Allocates the page for the new root. */
        void allocNewRootPage()
        {
            _pageNum = _tree->allocNewRootPage(*this);
        }

        /** \brief Reads page with the number \c pnum from the file to the current wrapper.
         *
         *  Requirements are similar to BaseBTree::readPage();
         */
        void readPage(UInt pnum)
        {
            _tree->readPage(pnum, _data);
            _pageNum = pnum;
        }

        /** \brief Loads the child page with number \c chNum of the page \c pw to the current wrapper.
         *
         *  If the cursor number is incorrects, throws an exception.
         */
        void readPageFromChild(PageWrapper& pw, UShort chNum);

        /** \brief Writes page into the file.
         *
         *  If the page number is not set, throws an exception.
         */
        void writePage();

#ifdef BTREE_WITH_REUSING_FREE_PAGES

        /** \brief Marks the page as free (for the following disk memory reusing). */
        void makePageFree()
        {
            _tree->markPageFree(_pageNum);
        }

#endif

        /**
         * \brief Writes the page into the Graphviz's DOT format into the given output stream \c ostream.
         *
         * \param ostream The given output stream.
         * \param code The code of the page.
         */
        void writeDot(std::ostream& ostream, std::string& code);

    public:
        
        /** \brief For the non-fulfilled current node splits its fulfilled child node with number \c iChild
         *  to the two nodes (pages).
         *
         *  \c iChild provides the cursor index which cannot be 0.
         *
         *  Throws an exception is the current node is fulfilled or the child node is not fulfilled.
         */
        void splitChild(UShort iChild);

    protected:

        PageWrapper(const PageWrapper&);

        PageWrapper& operator= (PageWrapper&);

    protected:

        /** \brief The raw data array. */
        Byte* _data;

        /** \brief The pointer to the tree. */
        BaseBTree* _tree;

        /** \brief Number of page in the file associated with the current wrapper. */
        UInt _pageNum;

    }; // class PageWrapper

    friend class PageWrapper;

    /** \brief Interface defining the two tree's keys comparing operation. */
    class IComparator {
    public:
        
        /** \brief Compares two keys: the left \c lhv and the right \c rhv.
         *
         *  (Max) keys size is defined by \c sz.
         *
         *  \returns true <tt>lhv < rhv<\tt>; otherwise false.
         *
         *  If keys cannot be compared because of some reason, throws std::invalid_argument.
         */
        virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) = 0;

        /** \brief Compares two keys.
          *
          * (Max) keys size is defined by \c sz.
          *
          * \returns true if two keys are equal; otherwise false.
          */
        virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) = 0;

    protected:

        ~IComparator() {};

    }; // class IComparator

    /** \brief Interface defining the key printing operation. */
    class IKeyPrinter {

    public:

        /** \brief Returns the string appearence of the given \c key.
         *
         * \param key The given key for printing.
         * \param sz The key's size (in bytes).
         * \returns The string appearence of the given \c key.
         */
        virtual std::string print(const Byte* key, UInt sz) = 0;

    protected:

        ~IKeyPrinter() {};

    }; // class IKeyPrinter

public:

    /** \brief Constructs new B-tree using received params.
    *
    *  Constructs new tree and writes it into \c stream. If file exists,
    *  it will be overwritten. If file cannot be open, throws an exception.
    *  \c order defines tree's order, \c recSize defines
    *  key's size (length) in bytes.
    */
    BaseBTree(UShort order, UShort recSize, IComparator* comparator, std::iostream* stream);

    /** \brief Constructs blank tree, params of this tree will be read from existing tree's file. */
    BaseBTree(IComparator* comparator, std::iostream* stream);

    /** \brief Destructor. */
    ~BaseBTree();

protected:

    BaseBTree(const BaseBTree&);

    BaseBTree& operator= (BaseBTree&);

public:

    /** \brief Creates the tree and its root page and writes them to the stream. */
    void createTree(UShort order, UShort recSize);

    /** \brief Loads the tree and its root page from the stream. */
    void loadTree();

    /** \brief Resets the tree's params. */
    void resetBTree();

    /** \brief Returns true if tree is opened, otherwise returns false. */
    bool isOpened() const { return _stream != nullptr && !_stream->fail(); } // = 0;

    /** \brief Reads page with number \c pnum from the file to the memory in \c dst.
     *
     *  If there is no such a page, throws an exception.
     */
    void readPage(UInt pnum, Byte* dst);

    /** \brief Writes to the file page with number \c pnum from the memory in \c dst.
     *
     *  If there is no such a page, throws an exception.
     */
    void writePage(UInt pnum, const Byte* dst);

    /** \brief Allocates the next page in the file and returns its number.
     *
     *
     *  \c keyNum defines the node's keys number, isLeaf defines whether new node should be leaf or not.
     *  If the stream is not ready, throws an exception.
     */
    UInt allocPage(PageWrapper& pw, UShort keysNum, bool isLeaf = false);

    /** \brief Allocates the page for the new tree's root. */
    UInt allocNewRootPage(PageWrapper& pw);

    /** \brief Inserts the key k into the tree using the ordering. */
    void insert(const Byte* k);

    /** \brief For the given key \c k finds the first its occurrence in the tree.
     *  If the key is found, returns the pointer to the appropriate bytes array, otherwise returns nullptr.
     */
    Byte* search(const Byte* k);

    /** \brief Searches the first occurrence of the key k recursively in the given page.
     *  \param k The key for searching.
     *  \param currentPage The given page.
     *  \param currentDepth The depth of the given page in the tree.
     *  \returns The Byte* array with the copy of the key k.
     */
    virtual Byte* search(const Byte* k, PageWrapper& currentPage, UInt currentDepth);

    /** \brief For the given key \c k finds all its occurrences in the tree and save them in the \c keys.
     *
     *  \returns The found elements count.
     */
    int searchAll(const Byte* k, std::list<Byte*>& keys);

    /**
     * \brief Searches all the occurrences of the key k recursively in the given page.
     * \param k The key for searching.
     * \param keys The list for saving the Byte* arrays with the copies of the key k.
     * \param currentPage The given page.
     * \param currentDepth The depth of the given page in the tree.
     * \returns The amount of all the occurrences of the key k in the given subtree.
     */
    virtual int searchAll(const Byte* k, std::list<Byte*>& keys, PageWrapper& currentPage, UInt currentDepth);

#ifdef BTREE_WITH_DELETION

    /** \brief For the given key \c k finds the first its occurrence in the tree
     *  and removes it from the tree.
     *
     *  \returns true if the key is removed, otherwise false.
     */    
    bool remove(const Byte* k);

    /** \brief For the given key \c k finds all its occurrences in the tree
     *  and removes them from the tree.
     *
     *  \returns The removed nodes count.
     */
    int removeAll(const Byte* k);

#endif

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    /**
     * \brief Marks the page with the given number as free (for the following disk memory reusing).
     * \param pageNum The given page number.
     * \throws std::invalid_argument if the given page number more than the last created page number.
     */
    void markPageFree(UInt pageNum);

#endif

    /** \brief Writes the tree into the Graphviz's DOT format into the given output stream \c ostream.
     *
     * \param ostream The given output stream.
     */
    void writeDot(std::ostream& ostream);

public:

    /** \brief Returns the tree's order. */
    UShort getOrder() const { return _order; }

    /** \brief Returns the max keys number in the node. Defined by the tree's order: <em>(2* _order - 1)<\em>. */
    UInt getMaxKeys() const { return _maxKeys; }

    /** \brief Returns the min keys number in the node. Defined by the tree's order: <em>(_order - 1)<\em>. */
    UInt getMinKeys() const { return _minKeys; }

    /** \brief Returns the keys area size defined as the max keys number multiplied by the key size. */
    UInt getKeysSize() const { return _keysSize; }

    /** \brief Returns the child nodes cursors area offset defined ad the keys area's end. */
    UInt getCursorsOfs() const { return _cursorsOfs; }

    /** \brief Returns the node (page) size. */
    UInt getNodePageSize() const { return _nodePageSize; }

    /** \brief Returns the key record size (length). */
    UShort getRecSize() const { return _recSize; }

    /** \brief Returns the last written page number (the written pages count). */
    UInt getLastPageNum() const { return _lastPageNum; }

    /** \brief Returns the unzero number of the tree's root page or 0 if there are no pages in the tree. */
    UInt getRootPageNum() const { return _rootPageNum;  }

    /** \brief Returns max depth reached during searching process. */
    UInt getMaxSearchDepth() const { return _maxSearchDepth; }

    /** \brief Returns the disk operations count during the last insert/search/remove operation. */
    UInt getDiskOperationsCount() const { return _diskOperationsCount; }

        /** \brief Sets the disk operations count during the last insert/search/remove operation to 0. */
    void resetDiskOperationsCount() { _diskOperationsCount = 0; }

     /** \brief Returns the reference to the current root page. */
    PageWrapper& getRootPage() { return _rootPage; }

     /** \brief The overloaded const getRootPage(). */
    const PageWrapper& getRootPage() const { return _rootPage; }

    /** \brief Sets the tree's comparator */
    void setComparator(IComparator* c) { _comparator = c; }

    /** \brief Returns the tree's comparator. */
    IComparator* getComparator() const { return _comparator; }

    IKeyPrinter* getKeyPrinter() const { return _keyPrinter; }

    void setKeyPrinter(IKeyPrinter* keyPrinter) { _keyPrinter = keyPrinter; }

    void setStream(std::iostream* s) { _stream  = s; }

protected:

    /** \brief Insert key k into the non-fulfilled node using the ordering.
     *
     *  If node is fulfilled, throws an exception.
     *  If comparator is not defined for the tree, throws an exception.
     */
    virtual void insertNonFull(const Byte* k, PageWrapper& currentNode);

    /**
     * \brief Inner child splitting method. It is necessary for reducing the number of the disk operations.
     *
     * \param iChild The number of child.
     * \param leftChild The left child wrapper.
     * \param rightChild The right child wrapper.
     */
    virtual void splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild);

#ifdef BTREE_WITH_DELETION

    /**
     * \brief Removes the first occurrence of the key k recursively in the given page.
     * \param k The key for removing.
     * \param currentPage The given page.
     * \returns true if the element is removed, false otherwise.
     */
    virtual bool remove(const Byte* k, PageWrapper& currentPage);

    /**
     * \brief Removes all the occurrences of the key k recursively in the given page.
     * \param k The key for removing.
     * \param currentPage The given page.
     * \returns The amount of all the occurrences of the key k in the given subtree.
     */
    int removeAll(const Byte* k, PageWrapper& currentPage);

    /**
     * \brief Removes the key with the given number recursively in the given page.
     * \param keyNum The given key number.
     * \param currentPage The given page.
     * \returns true if the element is removed, false otherwise.
     */
    virtual bool removeByKeyNum(UShort keyNum, PageWrapper& currentPage);

    /**
     * \brief Prepares subtree for removing in case 3 (when there is not the key k in the current page).
     * \param cursorNum The number of cursor to the subtree which potentially contains the key k.
     * \param currentPage The current page
     * \param child The child whose subtree potentially contains the key k.
     * \param leftNeighbour The instance for the left neighbour of the child.
     * \param rightNeighbour The instance for the right neighbour of the child.
     * \returns true if the child was merged with the left neighbour, false otherwise.
     */
    virtual bool prepareSubtree(UShort cursorNum, PageWrapper& currentPage, PageWrapper& child,
            PageWrapper& leftNeighbour, PageWrapper& rightNeighbour);

    /**
     * \brief Returns the max key in the given subtree and removes it recursively in the subtree.
     * \param pw The root of the subtree.
     * \returns The max key in the given subtree.
     */
    virtual const Byte* getAndRemoveMaxKey(PageWrapper& pw);

    /**
     * \brief Returns the min key in the given subtree and removes it recursively in the subtree.
     * \param pw The root of the subtree.
     * \returns The min key in the given subtree.
     */
    virtual const Byte* getAndRemoveMinKey(PageWrapper& pw);

    /**
     * \brief Merges the left child and the right child using the given median.
     * \param leftChild The left child.
     * \param rightChild The right child.
     * \param currentPage The parent of the left child and the right child.
     * \param medianNum The given median number in the parent.
     */
    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& rightChild,
            PageWrapper& currentPage, UShort medianNum);

    /**
     * \brief Moves the last key of the child's left neighbour to the median's place
     * and the median to the start of the child.
     * \param leftNeighbour The child's left neighbour.
     * \param child The child.
     * \param currentPage The child's parent.
     * \param childCursorNum The number of the parent's cursor to the child.
     */
    void moveOneKeyFromLeft(PageWrapper& leftNeighbour, PageWrapper& child,
            PageWrapper& currentPage, UShort childCursorNum);

    /**
     * \brief Moves the first key of the child's right neighbour to the median's place
     * and the median to the end of the child.
     * \param rightNeighbour The child's right neighbour.
     * \param child The child.
     * \param currentPage The child's parent.
     * \param childCursorNum The number of the parent's cursor to the child.
     */
    void moveOneKeyFromRight(PageWrapper& rightNeighbour, PageWrapper& child,
            PageWrapper& currentPage, UShort childCursorNum);

#endif

    virtual bool isFull(const PageWrapper& page) const;

    /** \brief Loads the tree's root page. */
    void loadRootPage();

    /** \brief Creates and writes the tree's root page and writes it to the stream. */
    void createRootPage();

    /** \brief Checks whether the stream is opened (the tree is ready) or not, If not, throws an exception. */
    void checkForOpenStream();

    /** \brief Writes the tree's header into the stream. */
    void writeHeader();

    /** \brief Read the tree's header from the stream. */
    void readHeader(Header& hdr);

    /** \brief Writes the written pages count into the stream. */
    void writePageCounter(); // UInt pc);

    /** \brief Reads the written pages count from the stream. */
    void readPageCounter();

    /** \brief Writes the tree's root page number into the stream. */
    void writeRootPageNum();

    /** \brief Reads the tree's root page number from the stream. */
    void readRootPageNum();

    /** \brief Sets the root page number.
     *
     *  If \c writeFlag == true, writes this number into the file immediately.
     */
    void setRootPageNum(UInt pnum, bool writeFlag = true);

    /** \brief Sets the tree's order and recalculates the associated values. */
    virtual void setOrder(UShort order, UShort recSize);

    /** \brief Reallocates the memory for the working pages. */
    void reallocWorkPages();

    /** \brief The inner part of the readPage(). */
    void readPageInternal(UInt pnum, Byte* dst);

    /** \brief The inner part of the writePage(). */
    void writePageInternal(UInt pnum, const Byte* dst);

    /** \brief Goes to the offset in the file matching to the page with number \c pnum. */
    void gotoPage(UInt pnum);

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    /**
     * \brief Allocate page using the free pages reusing concept.
     * \param pw PageWrapper for getting access to the allocated page.
     * \param keysNum The number of the keys in the allocated page.
     * \param isRoot Shows whether the allocated page is the root of the tree or not.
     * \param isLeaf Shows whether the allocated page is the leaf of the tree or not.
     * \returns The number of the allocated page.
     */
    UInt allocPageUsingFreePages(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf);

    /** \brief Loads the free pages counter from the disk. */
    void loadFreePagesCounter();

    /** \brief Writes the free pages counter to the disk. */
    void writeFreePagesCounter();

    /** \brief Gets the last free page number from the disk.
     *  \returns The last free page number.
     */
    UInt getLastFreePageNum();

    /**
     * \brief The internal part of the allocPageUsingFreePages() method.
     * \param pw PageWrapper for getting access to the allocated page.
     * \param keysNum The number of the keys in the allocated page.
     * \param isRoot Shows whether the allocated page is the root of the tree or not.
     * \param isLeaf Shows whether the allocated page is the leaf of the tree or not.
     * \param freePageNum The free page number for reusing.
     */
    void allocPageUsingFreePagesInternal(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf, UInt freePageNum);

    /**
     * \brief Gets the page offset from the begin of the file.
     * \param pageNum The page number for getting the offset.
     * \returns The page offset from the begin of the file.
     */
    ULong getPageOfs(UInt pageNum);

    /** \brief Returns the offset of the free pages numbers area from the begin of the file. */
    ULong getFreePagesInfoAreaOfs();

    /** \brief Returns the offset of the last free page number from the begin of the file. */
    ULong getLastFreePageNumOfs();

#endif

    /** \brief The internal part of the allocPage(). */
    UInt allocPageInternal(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf);

protected:

    /** \brief Defines the tree's order. */
    UShort _order;

    /** \brief The max keys number in the node. Defined by the tree's order: <em>(2* _order - 1)<\em>. */
    UInt _maxKeys;

    /** \brief The min keys number in the node. Defined by the tree's order: <em>(_order - 1)<\em>. */
    UInt _minKeys;

    /** \brief the keys area size defined as the max keys number multiplied by the key size. */
    UInt _keysSize;
    
    /** \brief The child nodes cursors area offset defined ad the keys area's end. */
    UInt _cursorsOfs;

    /** \brief The node (page) size. */
    UInt _nodePageSize;

    /** \brief The key record size (length). */
    UShort _recSize;

    /** \brief The last written page number (the written pages count). */
    UInt _lastPageNum;

    /** \brief The unzero number of the tree's root page or 0 if there are no pages in the tree. */
    UInt _rootPageNum;

    /** \brief The max reached tree's depth during the last search. */
    UInt _maxSearchDepth;

    /** \brief The disk operations count during the last insert/search/remove operation. */
    UInt _diskOperationsCount;

    /** \brief The stream into / from which the tree is written / read. */
    std::iostream* _stream;

    /** \brief The root page wrapper. Always stored in the memory. */
    PageWrapper _rootPage;

    /** \brief Comparator for the keys comparing. */
    IComparator* _comparator;

    IKeyPrinter* _keyPrinter;

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    /** \brief The free pages counter.
     *
     *  Contains the count of the free pages for reusing.
     */
    UInt _freePagesCounter;

#endif

}; // class BaseBTree

/** \brief The B+-tree. */
class BaseBPlusTree : public BaseBTree {

public:

    BaseBPlusTree(UShort order, UShort recSize, IComparator* comparator, std::iostream* stream)
            : BaseBTree(order, recSize, comparator, stream) { }

    BaseBPlusTree(IComparator* comparator, std::iostream* stream) : BaseBTree(comparator, stream) { }

    ~BaseBPlusTree();

protected:

    BaseBPlusTree(const BaseBPlusTree&);

    BaseBPlusTree& operator=(BaseBPlusTree&);

protected:

    virtual Byte* search(const Byte* k, PageWrapper& currentPage, UInt currentDepth) override;

    virtual int searchAll(const Byte* k, std::list<Byte*>& keys,
            PageWrapper& currentPage, UInt currentDepth) override;

#ifdef BTREE_WITH_DELETION

    virtual bool remove(const Byte* k, PageWrapper& currentPage) override;

    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& rightChild,
            PageWrapper& currentPage, UShort medianNum) override;

#endif

public:

    /**
     * \brief Returns the max keys number for the leaf node of the B+-tree.
     * \returns The max keys number for the leaf node of the B+-tree.
     */
    UInt getMaxLeafKeys() const { return _maxLeafKeys; }

    /**
     * \brief Returns the min keys number for the leaf node of the B+-tree.
     * \returns The min keys number for the leaf node of the B+-tree.
     */
    UInt getMinLeafKeys() const { return _minLeafKeys; }

protected:

    virtual void splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild) override;

    virtual void setOrder(UShort order, UShort recSize) override;

    virtual bool isFull(const PageWrapper& page) const override;

protected:

    /**
     * The max keys number for the leaf node of the B+-tree.
     */
    UInt _maxLeafKeys;

    /**
     * The min keys number for the leaf node of the B+-tree.
     */
    UInt _minLeafKeys;

}; // class BaseBPlusTree

/** \brief The B*-tree. */
class BaseBStarTree : public BaseBTree {

public:

    BaseBStarTree(UShort order, UShort recSize, IComparator* comparator, std::iostream* stream)
            : BaseBTree(order, recSize, comparator, stream) { }

    BaseBStarTree(IComparator* comparator, std::iostream* stream) : BaseBTree(comparator, stream) { }

    ~BaseBStarTree();

protected:

    BaseBStarTree(const BaseBStarTree&);

    BaseBStarTree& operator=(BaseBStarTree&);

protected:

#ifdef BTREE_WITH_DELETION

    virtual bool remove(const Byte* k, PageWrapper& currentPage) override;

    virtual bool prepareSubtree(UShort cursorNum, PageWrapper& currentPage, PageWrapper& child,
            PageWrapper& leftNeighbour, PageWrapper& rightNeighbour) override;

    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& rightChild,
            PageWrapper& currentPage, UShort medianNum) override;

    /**
     * \brief Merges the left child and the right child using the given median.
     * \param leftChild The left child.
     * \param middleChild The middle child.
     * \param rightChild The right child.
     * \param currentPage The parent of the left child, the middle child and the right child.
     * \param leftMedianNum The given left median number in the parent.
     * \param rightMedianNum The given right median number in the parent.
     */
    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& middleChild,
            PageWrapper& rightChild, PageWrapper& currentPage,
            UShort leftMedianNum, UShort rightMedianNum);

#endif

public:

    /**
     * \brief Returns the max keys number for the root node of the B*-tree.
     * \returns The max keys number for the root node of the B*-tree.
     */
    UInt getMaxRootKeys() const { return _maxRootKeys; }

    /**
     * \brief Returns the keys number for the left split product in the B*-tree.
     * \returns The keys number for the left split product in the B*-tree.
     */
    UInt getLeftSplitProductKeys() const { return _leftSplitProductKeys; }

    /**
     * \brief Returns the keys number for the middle split product in the B*-tree.
     * \returns The keys number for the middle split product in the B*-tree.
     */
    UInt getMiddleSplitProductKeys() const { return _middleSplitProductKeys; }

     /**
     * \brief Returns the keys number for the right split product in the B*-tree.
     * \returns The keys number for the right split product in the B*-tree.
     */
    UInt getRightSplitProductKeys() const { return _rightSplitProductKeys; }

    /**
     * \brief Returns the keys number for the middle split product in the B*-tree in the short split case
     * (sometimes when the sibling is not full).
     * \returns The keys number for the middle split product in the B*-tree in the short split case.
     */
    UInt getShortRightSplitProductKeys() const { return _shortRightSplitProductKeys; }

    UInt getRightSplitProductKeys(bool isShort) const { return isShort ? _shortRightSplitProductKeys : _rightSplitProductKeys; }

protected:

    virtual void insertNonFull(const Byte* k, PageWrapper& currentNode) override;

    virtual void splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild) override;

    /**
     * \brief Splits 2 children into the 3 children.
     *
     * \param node The parent node.
     * \param iLeft The index of the parent node's cursor to the left child.
     * \param left The left child.
     * \param middle The page that will contain the new middle child.
     * \param right The right child.
     */
    virtual void splitChildren(PageWrapper& node, UShort iLeft, PageWrapper& left,
            PageWrapper& middle, PageWrapper& right, bool isShort);

    virtual bool removeByKeyNum(UShort keyNum, PageWrapper& currentPage) override;

    /**
     * \brief Shares keys from the child to its left sibling to make this child non-full and inserts the key \param k.
     *
     * \param k The key for insertion.
     * \param node The parent node.
     * \param iChild The index of the parent node's cursor to the child.
     * \param child The child.
     * \param left The child's left sibling.
     */
    virtual bool shareKeysWithLeftChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
            PageWrapper& child, PageWrapper& left);

    /**
     * \brief Shares keys from the child to its right sibling to make this child non-full and inserts the key \param k.
     *
     * \param k The key for insertion.
     * \param node The parent node.
     * \param iChild The index of the parent node's cursor to the child.
     * \param child The child.
     * \param right The child's right sibling.
     */
    virtual bool shareKeysWithRightChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
            PageWrapper& child, PageWrapper& right);

    virtual void setOrder(UShort order, UShort recSize) override;

    virtual bool isFull(const PageWrapper& page) const override;

protected:

    /**
     * \brief The max keys number for the root node of the B*-tree.
     */
    UInt _maxRootKeys;

    /**
     * \brief The keys number for the left split product in the B*-tree.
     */
    UInt _leftSplitProductKeys;

    /**
     * \brief The keys number for the middle split product in the B*-tree.
     */
    UInt _middleSplitProductKeys;

    /**
     * \brief The keys number for the right split product in the B*-tree.
     */
    UInt _rightSplitProductKeys;

    /**
     * \brief The keys number for the middle split product in the B*-tree in the short split case
     * (sometimes when the sibling is not full).
     */
    UInt _shortRightSplitProductKeys;

}; // class BaseBStarTree

/** \brief The B*+-tree. */
class BaseBStarPlusTree : public BaseBStarTree {

public:

    BaseBStarPlusTree(UShort order, UShort recSize, IComparator* comparator, std::iostream* stream)
            : BaseBStarTree(order, recSize, comparator, stream) { }

    BaseBStarPlusTree(IComparator* comparator, std::iostream* stream) : BaseBStarTree(comparator, stream) { }

    ~BaseBStarPlusTree();

protected:

    BaseBStarPlusTree(const BaseBStarPlusTree&);

    BaseBStarPlusTree& operator=(BaseBStarPlusTree&);

protected:

    virtual Byte* search(const Byte* k, PageWrapper& currentPage, UInt currentDepth) override;

    virtual int searchAll(const Byte* k, std::list<Byte*>& keys,
            PageWrapper& currentPage, UInt currentDepth) override;

#ifdef BTREE_WITH_DELETION

    virtual bool remove(const Byte* k, PageWrapper& currentPage) override;

    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& rightChild,
            PageWrapper& currentPage, UShort medianNum) override;

    virtual void mergeChildren(PageWrapper& leftChild, PageWrapper& middleChild,
            PageWrapper& rightChild, PageWrapper& currentPage,
            UShort leftMedianNum, UShort rightMedianNum) override;

#endif

protected:

    virtual void splitChildren(PageWrapper& node, UShort iLeft, PageWrapper& left,
            PageWrapper& middle, PageWrapper& right, bool isShort) override;

    virtual bool shareKeysWithLeftChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
            PageWrapper& child, PageWrapper& left) override;

    virtual bool shareKeysWithRightChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
            PageWrapper& child, PageWrapper& right) override;

protected:

    virtual void splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild) override;

    virtual void setOrder(UShort order, UShort recSize) override;

    /**
     * \brief Sets the router key in the given page to the key with given number.
     * Uses for this the right key of the left child.
     * \param page The given page.
     * \param keyNum The given number of the key.
     */
    void setRouterKey(PageWrapper& page, UShort keyNum);

public:

    /**
     * \brief Returns the keys number for the middle split product in the B*+-tree in the leafs split case.
     * \returns The keys number for the middle split product in the B*+-tree in the leafs split case.
     */
    UInt getMiddleLeafSplitProductKeys() const { return _middleLeafSplitProductKeys; }

protected:

    /**
     * \brief The keys number for the middle split product in the B*+-tree in the leafs split case.
     */
    UInt _middleLeafSplitProductKeys;

}; // class BaseBStarPlusTree

/** \brief B-tree based on the file stream. */
class FileBaseBTree {

public:

    /** \brief Default constructor */
    FileBaseBTree() : FileBaseBTree(BaseBTree::TreeType::B_TREE) { }

    /** \brief Constructs new B-tree defined by the received params.
     *
     *  Constructs new tree and writes it to the file with name \c fileName. If file exists,
     *  it will be overwritten. If file cannot be open, throws an exception.
     */
    FileBaseBTree(UShort order, UShort recSize, BaseBTree::IComparator* comparator, const std::string& fileName)
            : FileBaseBTree(BaseBTree::TreeType::B_TREE, order, recSize, comparator, fileName) { }


    /** \brief Constructs the tree from existing tree's file.
     *
     *  If file cannot be opened, read or is incorrect, throws an exception.
     */
    FileBaseBTree(const std::string& fileName, BaseBTree::IComparator* comparator)
            : FileBaseBTree(BaseBTree::TreeType::B_TREE, fileName, comparator) { }

    /** \brief Constructs the multiway tree with received type. */
    FileBaseBTree(BaseBTree::TreeType treeType);

    /** \brief Constructs new multiway tree defined by the received params and type.
     *
     *  Constructs new tree and writes it to the file with name \c fileName. If file exists,
     *  it will be overwritten. If file cannot be open, throws an exception.
     */
    FileBaseBTree(BaseBTree::TreeType treeType, UShort order, UShort recSize,
            BaseBTree::IComparator* comparator, const std::string& fileName);

    /** \brief Constructs the tree by the received type from existing tree's file.
     *
     *  If file cannot be opened, read or is incorrect, throws an exception.
     */
    FileBaseBTree(BaseBTree::TreeType treeType, const std::string& fileName, BaseBTree::IComparator* comparator);

    /** \brief Destructor.
     *
     *  Closes the opened file streams.
     */
    ~FileBaseBTree();

protected:

    FileBaseBTree(const FileBaseBTree&);

    FileBaseBTree& operator= (FileBaseBTree&);

public:

    /** \brief Creates and opens inactive tree with the same params. If tree is already opened, throws an exception. */
    void create(UShort order, UShort recSize,
        const std::string& fileName);

    /** \brief Loads the tree from the file. If tree is already opened, throws an exception. */
    void open(const std::string& fileName);

    /** \brief Closes the opened tree and the streams. */
    void close();

public:

    /** \copydoc */
    bool isOpen() const;

    BaseBTree* getTree() const { return _tree; }

public:

    void insert(const Byte* k) { _tree->insert(k); }

    Byte* search(const Byte* k) { return _tree->search(k); }

    int searchAll(const Byte* k, std::list<Byte*>& keys) { return _tree->searchAll(k, keys); }

#ifdef BTREE_WITH_DELETION

    bool remove(const Byte* k) { return _tree->remove(k); }

    int removeAll(const Byte* k) { return _tree->removeAll(k); }

#endif

protected:

    /** \brief The internal part of create(). */
    void createInternal(UShort order, UShort recSize, // IComparator* comparator, 
        const std::string& fileName);

    /** \brief The internal part of open(). */
    void loadInternal(const std::string& fileName); // , IComparator* comparator);


    /** \brief The internal part of close(). */
    void closeInternal();

    /** \brief Checks the tree's params. If they are incorrect, throws an exception. */
    void checkTreeParams(UShort order, UShort recSize);

protected:

    /** \brief The tree's file name. */
    std::string _fileName;

    /** \brief The file stream storing the tree. */
    std::fstream _fileStream;

    BaseBTree* _tree = nullptr;

    bool isComposition = false;

}; // class FileBaseBTree

} // namespace btree

#endif // BTREE_WITH_REUSING_FREE_PAGES

#endif // BTREE_WITH_DELETION

#endif // BTREE_BTREE_H_
