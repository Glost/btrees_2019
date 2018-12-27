/// \file
/// \brief     B-tree / B+-tree / B*-tree / B*+-tree file indexer.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREEBASEDINDEX_INDEXER_H
#define BTREEBASEDINDEX_INDEXER_H

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <wchar.h>

#include "btree.h"
#include "utils.h"

namespace btree
{

/** \brief Represents the B-tree based indexer of the file records. */
class Indexer
{

public:

    /** \brief The max length of the stored name. */
    static const int NAME_LENGTH = 42;

#pragma pack(push, 1)
    /** \brief Represents the key for storing and searching. */
    struct Key
    {
        /** \brief Default constructor. */
        Key() : name(L""), offset(0) { }

        /** \brief Constructor.
         *
         * Assigns 0 to offset.
         * \param nam The name for storing and searching.
         */
        Key(const std::wstring& nam);

        /** \brief Constructor.
         *
         * \param nam The name for storing and searching.
         * \param ofs The offset of record from the file's begin.
         */
        Key(const std::wstring& nam, ULong ofs) : Key(nam) { offset = ofs; }

        /** \brief The name for storing and searching. */
        wchar_t name[NAME_LENGTH];

        /** \brief The offset of record from the file's begin. */
        ULong offset;
    }; // struct Key
#pragma pack(pop)

    /** \brief Represents the comparator of name part of the key. */
    struct NameComparator : public BaseBTree::IComparator
    {
        /** \brief Compares two name keys, returns true if the first name is lexicographically less than
         * the second name, false otherwise.
         *
         * \param lhv The first name key's byte array.
         * \param rhv The second name key's byte array.
         * \param sz The key's size.
         * \returns true if the first name is lexicographically less than the second name, false otherwise.
         */
        virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz);

        /** \brief Compares two name keys, returns true if they are equal, false otherwise.
         *
         *  \param lhv The first name key's byte array.
         *  \param rhv The second name key's byte array.
         *  \param sz The key's size.
         *  \returns true if the two name keys are equal, false otherwise.
         */
        virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz);
    }; // struct NameComparator

struct NameKeyPrinter : public BaseBTree::IKeyPrinter {

    virtual std::string print(const Byte* key, UInt sz) override;

}; // struct NameKeyPrinter

public:

    /** \brief Destructor.
     *
     *  Closes the tree and clears its memory.
     */
    ~Indexer();

public:

    /** \brief Creates the B-tree with given order.
     *
     *  \param order The order for creating the B-tree.
     *  \param treeFileName Name of the file for storing the tree.
     */
    void create(BaseBTree::TreeType treeType, UShort order, const std::string& treeFileName);

    void create(UShort order, const std::string& treeFileName) { create(BaseBTree::TreeType::B_TREE, order, treeFileName); }

    /** \brief Opens the stored B-tree.
     *
     *  \param treeFileName Name of the file where the tree is stored.
     */
    void open(BaseBTree::TreeType treeType, const std::string& treeFileName);

    void open(const std::string& treeFileName) { open(BaseBTree::TreeType::B_TREE, treeFileName); }

    /** \brief Closes the tree and clears its memory. */
    void close();

    /** \brief Performs indexing of the file: all the file's records are being stored in the B-tree during this process.
     *
     *  \param fileName Name of the file for indexing.
     *  \throws std::logic_error If the B-tree was not created or opened yet.
     *  \throws Other exception if some special problem during reading the file appeared.
     */
    void indexFile(const std::string& fileName);

    /** \brief Finds all the occurrences of the given name in the given file using the B-tree.
     *
     *  File should be indexed firstly.
     *  \param name Name for searching its occurrences.
     *  \param fileName Name of the file which records are being found.
     *  \returns List of the occurrences' strings.
     *  \throws std::logic_error If the B-tree was not created or opened or given file was not indexed yet.
     *  \throws Other exception if some special problem during reading the file appeared.
     */
    std::list<std::wstring> findAllOccurrences(const std::wstring& name, const std::string& fileName);

    /** \brief Returns tree's max depth reached during searching process.
     *
     *  \returns Tree's max depth reached during searching process.
     */
    UInt getMaxSearchDepth() { return _bt != nullptr ? _bt->getTree()->getMaxSearchDepth() : 0; }

    UInt getDiskOperationsCount() { return _bt != nullptr ? _bt->getTree()->getDiskOperationsCount() : 0; }

    void resetDiskOperationsCount() { if (_bt != nullptr) _bt->getTree()->resetDiskOperationsCount(); }

    FileBaseBTree* getTree() const { return _bt; }

private:

    /** \brief  Reads line from the file.
     *
     * \param   file The file.
     * \returns The line that was read from the file.
     */
    std::string getLine(std::ifstream& file);

private:

    /** \brief The B-tree. */
    FileBaseBTree* _bt = nullptr;

    /** \brief The comparator of name part of the key. */
    NameComparator _comparator;

    NameKeyPrinter _keyPrinter;

    /** \brief The name of the last indexed file.
     *
     * Used for checking if the file with given name was indexed.
     */
    std::string lastFileName;

}; // class Indexer

} // namespace btree

#endif // BTREEBASEDINDEX_INDEXER_H
