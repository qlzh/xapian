/** @file api_letor.cc
 * @brief test common features of API classes
 */
/* Copyright (C) 2007,2009,2012,2014,2015,2016 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_letor.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include <xapian.h>
#include <xapian-letor.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

static void
db_index_two_documents(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("Lions, Tigers, Bears and Giraffes", 1, "S");
    termgenerator.index_text("This paragraph talks about lions and tigers and "
			     "bears (oh, my!). It mentions giraffes, "
			     "but that's not really very important. Lions "
			     "and tigers are big cats, so they must be really "
			     "cuddly. Bears are famous for being cuddly, at "
			     "least when they're teddy bears.", 1, "XD");
    termgenerator.index_text("Lions, Tigers, Bears and Giraffes");
    termgenerator.increase_termpos();
    termgenerator.index_text("This paragraph talks about lions and tigers and "
			     "bears (oh, my!). It mentions giraffes, "
			     "but that's not really very important. Lions "
			     "and tigers are big cats, so they must be really "
			     "cuddly. Bears are famous for being cuddly, at "
			     "least when they're teddy bears.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Lions, Tigers and Bears", 1, "S");
    termgenerator.index_text("This is the paragraph of interest. Tigers are "
			     "massive beasts - I wouldn't want to meet a "
			     "hungry one anywhere. Lions are scary even when "
			     "lyin' down. Bears are scary even when bare. "
			     "Together I suspect they'd be less scary, as the "
			     "tigers, lions, and bears would all keep each "
			     "other busy. On the other hand, bears don't live "
			     "in the same continent as far as I know.", 1,
			     "XD");
    termgenerator.index_text("Lions, Tigers and Bears");
    termgenerator.increase_termpos();
    termgenerator.index_text("This is the paragraph of interest. Tigers are "
			     "massive beasts - I wouldn't want to meet a "
			     "hungry one anywhere. Lions are scary even when "
			     "lyin' down. Bears are scary even when bare. "
			     "Together I suspect they'd be less scary, as the "
			     "tigers, lions, and bears would all keep each "
			     "other busy. On the other hand, bears don't live "
			     "in the same continent as far as I know.");
    db.add_document(doc);
}


// To check for one document edge
static void
db_index_one_document(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("Tigers are solitary animals", 1, "S");
    termgenerator.index_text("Might be that only one Tiger is good enough to "
			     "Take out a ranker, a Tiger is a good way to "
			     "check if a test is working or Tiger not. Tiger. "
			     "What if the next line contains no Tigers? Would "
			     "it make a difference to your ranker ?  Tigers  "
			     "for the win.", 1, "XD");
    termgenerator.index_text("The will.");
    termgenerator.increase_termpos();
    termgenerator.index_text("Tigers would not be caught if one calls out the "
			     "Tiger from the dem. Well it stopped making sens "
			     "the best would be to stop now. But a tiger is "
			     "something you would not like to stop. ");
    db.add_document(doc);
}

// To check for three documents. out of which one is irrelavent
static void
db_index_three_documents(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("The will", 1, "S");
    termgenerator.index_text("The will are considered stop words in xapian and "
			     " would be thrown off, so the query I want to say "
			     " is score, yes, score. The Score of a game is  "
			     " the determining factor of a game, the score is "
			     " what matters at the end of the day. so my advise "
			     " to everyone is to Score it!. " , 1, "XD");
    termgenerator.index_text(" Score might be something else too, but this para "
			     " refers to score only at an abstract. Scores are "
			     " in general scorring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important. ");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Score score score score score score score", 1, "S");
    termgenerator.index_text("it might have an absurdly high rank in the qrel  "
			     " file or might have no rank at all in another.  "
			     " look out for this as a testcase, as might be edgy"
			     " Good luck and may this be with you. ", 1, "XD");
    termgenerator.index_text(" Another irrelavent paragraph to make sure the tf "
			     " values are down, but this increases the idf values "
			     " but let's see how this works out");
    termgenerator.increase_termpos();
    termgenerator.index_text("Nothing to do with the query. ");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("This document has nothing to do with score", 1, "S");
    termgenerator.index_text(" This is just to check if score is given a higher  "
			     " score if it is in the subject or not. Nothing "
			     " special, just juding scores by the look of it.  "
			     " some more scores but a bad qrel should be enough "
			     "to make sure it is ranked down" , 1, "XD");
    termgenerator.index_text(" Score might be something else too, but this para "
			     " refers to score only at an abstract. Scores are "
			     " in general scorring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important. ");
    db.add_document(doc);
}

DEFINE_TESTCASE(createfeaturevector, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker1",
				       db_index_two_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mset;
    vector<Xapian::FeatureVector> fv =
	    fl.create_feature_vectors(mset, Xapian::Query("lions"), db);
    TEST(fv.empty());
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    TEST_EQUAL(mset.size(), 2);
    fv = fl.create_feature_vectors(mset, Xapian::Query("lions"), db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    TEST_EQUAL(fv[1].get_fcount(), 19);
    return true;
}

DEFINE_TESTCASE(createfeaturevectoronevector, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker2",
				       db_index_one_document);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mset;
    vector<Xapian::FeatureVector> fv =
	    fl.create_feature_vectors(mset, Xapian::Query("tigers"), db);
    TEST(fv.empty());
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    fv = fl.create_feature_vectors(mset, Xapian::Query("tigers"), db);
    TEST_EQUAL(fv.size(), 1);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    return true;
}

DEFINE_TESTCASE(createfeaturevectoronevector_wrongquery, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker3",
				       db_index_one_document);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("llamas"));
    Xapian::MSet mset;
    vector<Xapian::FeatureVector> fv =
	    fl.create_feature_vectors(mset, Xapian::Query("llamas"), db);
    TEST(fv.empty());

    mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
    fv = fl.create_feature_vectors(mset, Xapian::Query("llamas"), db);
    TEST_EQUAL(fv.size(), 0);
    return true;
}

DEFINE_TESTCASE(createfeaturevectorthree, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker4",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    vector<Xapian::FeatureVector> fv =
	    fl.create_feature_vectors(mset, Xapian::Query("score"), db);
    TEST(fv.empty());

    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    fv = fl.create_feature_vectors(mset, Xapian::Query("score"), db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    TEST_EQUAL(fv[1].get_fcount(), 19);
    return true;
}


DEFINE_TESTCASE(preparetrainingfileonedb, generated)
{
    string db_path = get_database_path("apitest_listnet_ranker1",
				       db_index_one_document);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_doc.txt";
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_data_one_doc.txt");
    ifstream if1(training_data);
    ifstream if2("training_output0.txt");
    string file1;
    string file2;
    while (getline(if1, file1)) {
	TEST(getline(if2, file2));
	istringstream iss1(file1);
	istringstream iss2(file2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST(abs(stod(temp1.substr(t1 + 1)) -
			 stod(temp2.substr(t2 + 1))) <
		     0.001);
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, file2));

    return true;
}

DEFINE_TESTCASE(preparetrainingfile, generated)
{
    string db_path = get_database_path("apitest_listnet_ranker2",
				       db_index_two_documents);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output1.txt");
    ifstream if1(training_data);
    ifstream if2("training_output1.txt");
    string file1;
    string file2;
    while (getline(if1, file1)) {
	TEST(getline(if2, file2));
	istringstream iss1(file1);
	istringstream iss2(file2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST(abs(stod(temp1.substr(t1 + 1)) -
			 stod(temp2.substr(t2 + 1))) <
		     0.001);
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, file2));

    return true;
}


DEFINE_TESTCASE(preparetrainingfilethree_missingQrel, generated)
{
    string db_path = get_database_path("apitest_listnet_ranker3	",
				       db_index_three_documents);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_onemiss.txt";
    string training_data = data_directory + "training_data_three_onemiss.txt";
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_three_miss.txt");
    ifstream if1(training_data);
    ifstream if2("training_output_three_miss.txt");
    string file1;
    string file2;
    while (getline(if1, file1)) {
	TEST(getline(if2, file2));
	istringstream iss1(file1);
	istringstream iss2(file2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST(abs(stod(temp1.substr(t1 + 1)) -
			 stod(temp2.substr(t2 + 1))) <
		     0.001);
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, file2));

    return true;
}

DEFINE_TESTCASE(preparetrainingfilethree, generated)
{
    string db_path = get_database_path("apitest_listnet_ranker4",
				       db_index_three_documents);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_three_correct.txt");
    ifstream if1(training_data);
    ifstream if2("training_output_three_correct.txt");
    string file1;
    string file2;
    while (getline(if1, file1)) {
	TEST(getline(if2, file2));
	istringstream iss1(file1);
	istringstream iss2(file2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST(abs(stod(temp1.substr(t1 + 1)) -
			 stod(temp2.substr(t2 + 1))) <
		     0.001);
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, file2));

    return true;
}


////////// Running rankers ////////////////

DEFINE_TESTCASE(listnet_ranker, generated)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listnet_ranker",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output.txt", 10));
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output.txt", 10);
    ranker.score(query, qrel, "ListNet_Ranker", "err_output.txt", 10, "ERRScore");

    return true;
}


DEFINE_TESTCASE(listnet_ranker_one_file, generated)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listnet_ranker5",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output1.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output1.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output1.txt", 10));
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output1.txt", 10);
    ranker.score(query, qrel, "ListNet_Ranker", "err_output1.txt", 10, "ERRScore");

    return true;
}


// DEFINE_TESTCASE(listnet_ranker_three_miss, generated)
// {
//     Xapian::ListNETRanker ranker;
//     TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
//     string db_path = get_database_path("apitest_listnet_ranker3",
//					   db_index_three_documents);
//     Xapian::Enquire enquire((Xapian::Database(db_path)));
//     enquire.set_query(Xapian::Query("score"));
//     Xapian::MSet mymset = enquire.get_mset(0, 10);
//     string data_directory = test_driver::get_srcdir() + "/testdata/";
//     string query = data_directory + "querythree.txt";
//     string qrel = data_directory + "qrelthree_onemiss.txt";
//     string training_data = data_directory + "training_data_three_onemiss.txt";
//     ranker.set_database_path(db_path);
//     TEST_EQUAL(ranker.get_database_path(), db_path);
//     ranker.set_query(Xapian::Query("score"));
//     ranker.train_model(training_data);
//     TEST_EQUAL(mymset.size(), 2);
//     Xapian::docid doc1 = *mymset[0];
//     Xapian::docid doc2 = *mymset[1];
//     ranker.rank(mymset);
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     mymset = enquire.get_mset(0, 10);
//     ranker.train_model(training_data, "ListNet_Ranker");
//     ranker.rank(mymset, "ListNet_Ranker");
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     TEST_EXCEPTION(Xapian::LetorInternalError,
// 			ranker.score(query, qrel, "ListNet_Ranker",
// 			"scorer_output2.txt", 10, ""));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
// 				ranker.score("", qrel, "ListNet_Ranker",
// 				"scorer_output2.txt", 10));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
// 			ranker.score(qrel, "", "ListNet_Ranker",
// 						"scorer_output.txt2", 10));
//     ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output2.txt", 10);
// 	ranker.score(query, qrel, "ListNet_Ranker", "err_output2.txt", 10, "ERRScore");

//     return true;
// }

DEFINE_TESTCASE(listnet_ranker_three_correct, generated)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listnet_ranker6",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    // Xapian::docid doc3 = *mymset[2];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    // TEST_EQUAL(doc3, *mymset[2]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    // TEST_EQUAL(doc3, *mymset[2]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output3.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output3.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output3.txt", 10));
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output3.txt", 10);
    ranker.score(query, qrel, "ListNet_Ranker", "err_output3.txt", 10, "ERRScore");

    return true;
}

/// Svn ranker check


DEFINE_TESTCASE(svm_ranker, generated)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_svm_ranker",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output.txt", 10));
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output.txt", 10);
    ranker.score(query, qrel, "SVM_Ranker", "err_output.txt", 10, "ERRScore");

    return true;
}


DEFINE_TESTCASE(svm_ranker_one_file, generated)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_svm_ranker1",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output1.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output1.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output1.txt", 10));
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output1.txt", 10);
    ranker.score(query, qrel, "SVM_Ranker", "err_output1.txt", 10, "ERRScore");

    return true;
}


// DEFINE_TESTCASE(svm_ranker_three_miss, generated)
// {
//     Xapian::SVMRanker ranker;
//     TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
//     string db_path = get_database_path("apitest_svm_ranker3",
//				       db_index_three_documents);
//     Xapian::Enquire enquire((Xapian::Database(db_path)));
//     enquire.set_query(Xapian::Query("score"));
//     Xapian::MSet mymset = enquire.get_mset(0, 10);
//     string data_directory = test_driver::get_srcdir() + "/testdata/";
//     string query = data_directory + "querythree.txt";
//     string qrel = data_directory + "qrelthree_onemiss.txt";
//     string training_data = data_directory + "training_data_three_onemiss.txt";
//     ranker.set_database_path(db_path);
//     TEST_EQUAL(ranker.get_database_path(), db_path);
//     ranker.set_query(Xapian::Query("score"));
//     ranker.train_model(training_data);
//     TEST_EQUAL(mymset.size(), 2);
//     Xapian::docid doc1 = *mymset[0];
//     Xapian::docid doc2 = *mymset[1];
//     ranker.rank(mymset);
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     mymset = enquire.get_mset(0, 10);
//     ranker.train_model(training_data, "SVM_Ranker");
//     ranker.rank(mymset, "SVM_Ranker");
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     TEST_EXCEPTION(Xapian::LetorInternalError,
//		   ranker.score(query, qrel, "SVM_Ranker",
//				"scorer_output2.txt", 10, ""));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
//		   ranker.score("", qrel, "SVM_Ranker",
//				"scorer_output2.txt", 10));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
//		   ranker.score(qrel, "", "SVM_Ranker",
//				"scorer_output.txt2", 10));
//     ranker.score(query, qrel, "SVM_Ranker", "ndcg_output2.txt", 10);
//     ranker.score(query, qrel, "SVM_Ranker", "err_output2.txt", 10, "ERRScore");

//     return true;
// }

DEFINE_TESTCASE(svm_ranker_three_correct, generated)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_svm_ranker2",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    // Xapian::docid doc3 = *mymset[2];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    // TEST_EQUAL(doc3, *mymset[2]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    // TEST_EQUAL(doc3, *mymset[2]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output3.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output3.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output3.txt", 10));
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output3.txt", 10);
    ranker.score(query, qrel, "SVM_Ranker", "err_output3.txt", 10, "ERRScore");

    return true;
}

// listmle ranker check /////


DEFINE_TESTCASE(listmle_ranker, generated)
{
    Xapian::ListMLERanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listmle_ranker",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output.txt", 10));
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output.txt", 10);
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output.txt", 10, "ERRScore");

    return true;
}



DEFINE_TESTCASE(listmle_ranker_one_file, generated)
{
    Xapian::ListMLERanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listmle_ranker1",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output1.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output1.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output1.txt", 10));
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output1.txt", 10);
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output1.txt", 10, "ERRScore");

    return true;
}


// DEFINE_TESTCASE(listmle_ranker_three_miss, generated)
// {
//     Xapian::ListMLERanker ranker;
//     TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
//     string db_path = get_database_path("apitest_listmle_ranker3",
// 				       db_index_three_documents);
//     Xapian::Enquire enquire((Xapian::Database(db_path)));
//     enquire.set_query(Xapian::Query("score"));
//     Xapian::MSet mymset = enquire.get_mset(0, 10);
//     string data_directory = test_driver::get_srcdir() + "/testdata/";
//     string query = data_directory + "querythree.txt";
//     string qrel = data_directory + "qrelthree_onemiss.txt";
//     string training_data = data_directory + "training_data_three_onemiss.txt";
//     ranker.set_database_path(db_path);
//     TEST_EQUAL(ranker.get_database_path(), db_path);
//     ranker.set_query(Xapian::Query("score"));
//     ranker.train_model(training_data);
//     TEST_EQUAL(mymset.size(), 2);
//     Xapian::docid doc1 = *mymset[0];
//     Xapian::docid doc2 = *mymset[1];
//     ranker.rank(mymset);
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     mymset = enquire.get_mset(0, 10);
//     ranker.train_model(training_data, "ListMLE_Ranker");
//     ranker.rank(mymset, "ListMLE_Ranker");
//     TEST_EQUAL(doc1, *mymset[1]);
//     TEST_EQUAL(doc2, *mymset[0]);
//     TEST_EXCEPTION(Xapian::LetorInternalError,
// 		   ranker.score(query, qrel, "ListMLE_Ranker",
// 				"scorer_output2.txt", 10, ""));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
// 		   ranker.score("", qrel, "ListMLE_Ranker",
// 				"scorer_output2.txt", 10));
//     TEST_EXCEPTION(Xapian::FileNotFoundError,
// 		   ranker.score(qrel, "", "ListMLE_Ranker",
// 				"scorer_output.txt2", 10));
//     ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output21.txt", 10);
//     ranker.score(query, qrel, "ListMLE_Ranker", "err_output21.txt", 10, "ERRScore");

//     return true;
// }

DEFINE_TESTCASE(listmle_ranker_three_correct, generated)
{
    Xapian::ListMLERanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listmle_ranker2",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(mymset.size(), 2);

    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output3.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output3.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output3.txt", 10));
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output3.txt", 10);
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output3.txt", 10, "ERRScore");

    return true;
}


//// Featurename check
DEFINE_TESTCASE(featurename, !backend)
{
    Xapian::TfDoclenCollTfCollLenFeature feature1;
    Xapian::TfDoclenFeature feature2;
    Xapian::IdfFeature feature3;
    Xapian::TfFeature feature4;
    Xapian::TfIdfDoclenFeature feature5;
    Xapian::CollTfCollLenFeature feature6;
    TEST_EQUAL(feature1.name(), "TfDoclenCollTfCollLenFeature");
    TEST_EQUAL(feature2.name(), "TfDoclenFeature");
    TEST_EQUAL(feature3.name(), "IdfFeature");
    TEST_EQUAL(feature4.name(), "TfFeature");
    TEST_EQUAL(feature5.name(), "TfIdfDoclenFeature");
    TEST_EQUAL(feature6.name(), "CollTfCollLenFeature");

    return true;
}

DEFINE_TESTCASE(err_scorer, !backend)
{
    /* Derived from the example mentioned in the blogpost
     * https://lingpipe-blog.com/2010/03/09/chapelle-metzler-zhang-grinspan-2009-expected-reciprocal-rank-for-graded-relevance/
     */
    vector<Xapian::FeatureVector> fvv;
    Xapian::FeatureVector temp1;
    Xapian::FeatureVector temp2;
    Xapian::FeatureVector temp3;
    temp1.set_label(3);
    fvv.push_back(temp1);
    temp2.set_label(2);
    fvv.push_back(temp2);
    temp3.set_label(4);
    fvv.push_back(temp3);
    Xapian::ERRScore err;
    double err_score = err.score(fvv);

    TEST(abs(err_score - 0.63) < 0.01);

    return true;
}
