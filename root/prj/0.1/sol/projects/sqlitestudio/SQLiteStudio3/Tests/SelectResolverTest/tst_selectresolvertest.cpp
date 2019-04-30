#include "selectresolver.h"
#include "db/db.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/parser.h"
#include "dbsqlite3mock.h"
#include "mocks.h"
#include "parser/parser.h"
#include <QString>
#include <QtTest>
#include <QSet>

class SelectResolverTest : public QObject
{
        Q_OBJECT

    public:
        SelectResolverTest();

    private:
        Db* db = nullptr;

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testTableHash();
        void testColumnHash();
        void testWithCommonTableExpression();
        void testWithCte2();
        void testStarWithJoinAndError();
        void testTableFunction();
        void test1();
        void testSubselectWithAlias();
};

SelectResolverTest::SelectResolverTest()
{
}

void SelectResolverTest::testTableHash()
{
    QSet<SelectResolver::Table> tables;

    SelectResolver::Table t1;
    t1.database = "d1";
    t1.table = "t1";
    t1.tableAlias = "a1";
    tables << t1;

    // different alias and database
    SelectResolver::Table t2;
    t2.database = "d2";
    t2.table = "t1";
    t2.tableAlias = QString::null;
    tables << t2;

    // different database
    SelectResolver::Table t3;
    t3.database = "d2";
    t3.table = "t1";
    t3.tableAlias = "a1";
    tables << t3;

    // same as t3
    SelectResolver::Table t4;
    t4.database = "d2";
    t4.table = "t1";
    t4.tableAlias = "a1";
    tables << t4;

    // all null
    SelectResolver::Table t5;
    tables << t5;

    // same as t5
    SelectResolver::Table t6;
    tables << t6;

    // similar to t1, but different database
    SelectResolver::Table t7;
    t7.database = "x";
    t7.table = "t1";
    t7.tableAlias = "a1";
    tables << t7;

    // similar to t1, but different table
    SelectResolver::Table t8;
    t8.database = "d1";
    t8.table = "x";
    t8.tableAlias = "a1";
    tables << t8;

    // similar to t1, but different alias
    SelectResolver::Table t9;
    t9.database = "d1";
    t9.table = "t1";
    t9.tableAlias = "x";
    tables << t9;

    QVERIFY(tables.size() == 7);
}

void SelectResolverTest::testColumnHash()
{
    QSet<SelectResolver::Column> columns;

    SelectResolver::Column c1;
    c1.database = "d1";
    c1.table = "t1";
    c1.column = "c1";
    c1.alias = "a1";
    c1.tableAlias = "ta1";
    c1.displayName = "d1";
    c1.type = SelectResolver::Column::COLUMN;
    columns << c1;

    // This should be treated as equal to c1.
    SelectResolver::Column c2;
    c2.database = "d1";
    c2.table = "t1";
    c2.column = "c1";
    c2.alias = "x";
    c2.tableAlias = "ta1";
    c2.displayName = "x";
    c2.type = SelectResolver::Column::OTHER;
    columns << c2;

    // Different database
    SelectResolver::Column c3;
    c3.database = "x";
    c3.table = "t1";
    c3.column = "c1";
    c3.alias = "x";
    c3.tableAlias = "ta1";
    c3.displayName = "x";
    c3.type = SelectResolver::Column::OTHER;
    columns << c3;

    // Different table
    SelectResolver::Column c4;
    c4.database = "d1";
    c4.table = "x";
    c4.column = "c1";
    c4.alias = "x";
    c4.tableAlias = "ta1";
    c4.displayName = "x";
    c4.type = SelectResolver::Column::OTHER;
    columns << c4;

    // Different column
    SelectResolver::Column c5;
    c5.database = "d1";
    c5.table = "t1";
    c5.column = "x";
    c5.alias = "x";
    c5.tableAlias = "ta1";
    c5.displayName = "x";
    c5.type = SelectResolver::Column::OTHER;
    columns << c5;

    // Different table alias
    SelectResolver::Column c6;
    c6.database = "d1";
    c6.table = "t1";
    c6.column = "c1";
    c6.alias = "x";
    c6.tableAlias = "x";
    c6.displayName = "x";
    c6.type = SelectResolver::Column::OTHER;
    columns << c6;

    QVERIFY(columns.size() == 5);
}

void SelectResolverTest::testWithCommonTableExpression()
{
    // Test with query from examples from SQLite documentation
    QString sql = "WITH RECURSIVE works_for_alice(n) AS ("
                  "               VALUES('Alice')"
                  "               UNION"
                  "               SELECT name"
                  "                 FROM org, works_for_alice"
                  "                WHERE org.boss = works_for_alice.n"
                  "     )"
                  "     SELECT avg(height)"
                  "       FROM org"
                  "      WHERE org.name IN works_for_alice";

    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column>> columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QList<SelectResolver::Column> coreColumns = columns.first();
    QVERIFY(coreColumns.size() == 1);
    QVERIFY(coreColumns[0].type == SelectResolver::Column::OTHER);
    QVERIFY(coreColumns[0].flags & SelectResolver::Flag::FROM_CTE_SELECT);
}

void SelectResolverTest::testWithCte2()
{
    QString sql = "with m(c1, c2) as ("
                  "     values (1, 'a'), (2, 'b'), (3, 'c')"
                  ")"
                  "select * from m";

    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column>> columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QList<SelectResolver::Column> coreColumns = columns.first();
    QVERIFY(coreColumns.size() == 2);
    QVERIFY(coreColumns[0].type == SelectResolver::Column::COLUMN);
    QVERIFY(coreColumns[0].flags & SelectResolver::Flag::FROM_CTE_SELECT);
    QVERIFY(coreColumns[1].type == SelectResolver::Column::COLUMN);
    QVERIFY(coreColumns[1].flags & SelectResolver::Flag::FROM_CTE_SELECT);
}

void SelectResolverTest::testStarWithJoinAndError()
{
    QString sql = "SELECT t1.*, t2.* FROM test t1 JOIN test2 USING (col1)";
    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column> > columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QVERIFY(columns.first().size() == 3);
    QVERIFY(resolver.hasErrors());
}

void SelectResolverTest::testTableFunction()
{
    QString sql = "select * from json_tree(json_array(1, 2, 3))";
    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    SqlQueryPtr versionResult = db->exec("select sqlite_version()");
    qDebug() << "SQLite3 version:" << versionResult->getSingleCell().toString();

    SqliteSelectPtr select = parser.getQueries().first().dynamicCast<SqliteSelect>();
    QList<QList<SelectResolver::Column> > columns = resolver.resolve(select.data());
    if (resolver.hasErrors()) {
        for (const QString& err : resolver.getErrors())
            qWarning() << err;
    }
    QVERIFY(!resolver.hasErrors());
    QVERIFY(columns.first().size() == 8);
    QVERIFY(columns.first().first().type == SelectResolver::Column::OTHER);
}

void SelectResolverTest::test1()
{
    QString sql = "SELECT * FROM (SELECT count(col1), col2 FROM test)";
    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column> > columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QList<SelectResolver::Column> coreColumns = columns.first();
    QVERIFY(coreColumns[0].type == SelectResolver::Column::OTHER);
    QVERIFY(coreColumns[1].type == SelectResolver::Column::COLUMN);
    QVERIFY(coreColumns[1].table == "test");
    QVERIFY(coreColumns[1].column == "col2");
}

void SelectResolverTest::testSubselectWithAlias()
{
    QString sql = "SELECT * FROM (SELECT m.col1, m.col2, secm.col3 FROM test AS m LEFT OUTER JOIN test AS secm ON secm.col1 = m.col2) alias3;";
    SelectResolver resolver(db, sql);
    Parser parser(db->getDialect());
    QVERIFY(parser.parse(sql));

    QList<QList<SelectResolver::Column> > columns = resolver.resolve(parser.getQueries().first().dynamicCast<SqliteSelect>().data());
    QList<SelectResolver::Column> coreColumns = columns.first();
    QVERIFY(coreColumns[0].tableAlias == "alias3");
    QVERIFY(coreColumns[1].tableAlias == "alias3");
    QVERIFY(coreColumns[2].tableAlias == "alias3");
    QVERIFY(coreColumns[0].oldTableAliases.size() == 1);
    QVERIFY(coreColumns[1].oldTableAliases.size() == 1);
    QVERIFY(coreColumns[2].oldTableAliases.size() == 1);
    QVERIFY(coreColumns[0].oldTableAliases.first() == "m");
    QVERIFY(coreColumns[1].oldTableAliases.first() == "m");
    QVERIFY(coreColumns[2].oldTableAliases.first() == "secm");
}

void SelectResolverTest::initTestCase()
{
    initKeywords();
    Lexer::staticInit();
    initMocks();

    db = new DbSqlite3Mock("testdb");
    db->open();
    db->exec("CREATE TABLE test (col1, col2, col3);");
    db->exec("CREATE TABLE org (name TEXT PRIMARY KEY, boss TEXT REFERENCES org, height INT)");
    db->exec("CREATE TABLE test2 (col1);");
    //SqlQueryPtr results = db->exec("SELECT name FROM sqlite_master");
}

void SelectResolverTest::cleanupTestCase()
{
    db->close();
    delete db;
    db = nullptr;
}

QTEST_APPLESS_MAIN(SelectResolverTest)

#include "tst_selectresolvertest.moc"
