/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * GTestRunner.cpp - Created on 2010-07-23                                                   *
 *                                                                                           *
 * Copyright (C) 2010 Sandy Chapman                                                          *
 *                                                                                           *
 * This library is free software; you can redistribute it and/or modify it under the         *
 * terms of the GNU Lesser General Public License as published by the Free Software          *
 * Foundation; either version 2.1 of the License, or (at your option) any later version.     *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; *
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. *
 * See the GNU Lesser General Public License for more details.                               *
 * You should have received a copy of the GNU Lesser General Public License along with this  *
 * library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, *
 * Boston, MA 02111-1307 USA                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "GTestRunner.h"
#include "TestTreeWidgetItem.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

GTestRunner::GTestRunner(QWidget *parent, Qt::WFlags flags)
 : QMainWindow(parent, flags),
   fileMenu(tr("&File")), helpMenu(tr("&Help")), statusBar(this),
   centralWidget(this), testTreeTools(this), testTree(this),
   gtestList()
{
	resize(500, 800);
	setup();
}

GTestRunner::~GTestRunner()
{

}

void GTestRunner::setup() {
	//menus
	menuBar = QMainWindow::menuBar();
	setupMenus();
	setMenuBar(menuBar);

	//toolbar
	setupToolBars();

	setCentralWidget(&centralWidget);

	QObject::connect(&testTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
					 this, SLOT(treeItemClicked(QTreeWidgetItem*, int)));

	setupLayout();
}

void GTestRunner::setupMenus() {
	QAction* newTestSetupAct = new QAction(tr("&New Test Setup..."), this);
	QAction* openTestSetupAct = new QAction(tr("&Open Test Setup..."), this);
	QAction* saveTestSetupAct = new QAction(tr("&Save Test Setup..."), this);
	QAction* importTestExeAct = new QAction(tr("&Import Test Executable"), this);
	QAction* quitAct = new QAction(tr("&Exit"), this);

	QObject::connect(importTestExeAct, SIGNAL(triggered()),
					 this, SLOT(addTests()));

	fileMenu.addAction(newTestSetupAct);
	fileMenu.addAction(openTestSetupAct);
	fileMenu.addAction(saveTestSetupAct);
	fileMenu.addAction(importTestExeAct);
	fileMenu.addAction(quitAct);

	QAction* aboutAct = new QAction(tr("&About GTestRunner..."), this);
	QAction* aboutQtAct = new QAction(tr("About &Qt..."), this);

	helpMenu.addAction(aboutAct);
	helpMenu.addAction(aboutQtAct);

	menuBar->addMenu(&fileMenu);
	menuBar->addMenu(&helpMenu);
}

void GTestRunner::setupToolBars() {
	QAction* runTestsAct = new QAction(tr("Run"), this);
	QAction* stopTestsAct = new QAction(tr("Stop"), this);
	QAction* addTestsAct = new QAction(tr("Add"), this);
	QAction* removeTestsAct = new QAction(tr("Remove"), this);

	QObject::connect(runTestsAct, SIGNAL(triggered()),
					 this, SLOT(runTests()));

	QObject::connect(addTestsAct, SIGNAL(triggered()),
					 this, SLOT(addTests()));

	testTreeTools.addAction(runTestsAct);
	testTreeTools.addAction(stopTestsAct);
	testTreeTools.addAction(addTestsAct);
	testTreeTools.addAction(removeTestsAct);
}

void GTestRunner::setupLayout() {
	QGroupBox *treeBox = new QGroupBox(tr("Unit Tests Loaded"));
	QVBoxLayout *treeLayout = new QVBoxLayout;
	treeLayout->addWidget(&testTreeTools);
	treeLayout->addWidget(&testTree);
	treeBox->setLayout(treeLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(treeBox);

	centralWidget.setLayout(mainLayout);
}

void GTestRunner::addTests() {
	bool addResolved = false;
	QString filepath;
	GTestExecutable* newTest = new GTestExecutable(this);
	while(!addResolved) {
		filepath = QFileDialog::getOpenFileName(this, tr("Select Google Test Executable"));
		qDebug() << "File path received:" << filepath;
		if(filepath.isEmpty())
			return;

		newTest->setExecutablePath(filepath);
		GTestExecutable::STATE state = newTest->getState();
		switch(state) {
			case GTestExecutable::FILE_NOT_FOUND: {
				QMessageBox::StandardButton btnPressed = QMessageBox::warning(this,
						tr("File Not Found"),
						tr("It appears the filename entered does not exist. Please select a valid Google test executable."),
						QMessageBox::Retry | QMessageBox::Abort);
				switch(btnPressed) {
					case QMessageBox::Retry:
						continue; //go back to top of loop.
					case QMessageBox::Abort: default:
						return;   //exit attempt to add a test executable
					}
			break;
			}
			case GTestExecutable::INSUFFICIENT_PRIVILEGES: {
				QMessageBox::StandardButton btnPressed = QMessageBox::warning(this,
						tr("Insufficient Permissions"),
						tr("It appears that you do not have sufficient privileges to execute this file. \
						   GTestRunner can attempt to reset the permissions to enable executing this test file. \
						   \nWould you like GTestRunner to attempt this?"),
						QMessageBox::Yes | QMessageBox::No);
				switch(btnPressed) {
					case QMessageBox::Yes:
						continue;
					case QMessageBox::No: default: {
						QMessageBox::StandardButton btnPressed = QMessageBox::question(this,
							tr("Try Again?"),
							tr("Would you like to try again and select another file?"),
							QMessageBox::Yes | QMessageBox::No);
						switch(btnPressed) {
							case QMessageBox::Yes:
								continue;
							case QMessageBox::No: default:
								return;
						}
					break;
					}
				}
				break;
			}
			case GTestExecutable::VALID: {
				addResolved = true;
				break;
			}
		}
	}

	invokeListingRetrieval(filepath);
}

void GTestRunner::invokeListingRetrieval(QString filepath) {
	GTestExecutable *gtest = new GTestExecutable(this, filepath);
	QObject::connect(gtest, SIGNAL(listingReady(GTestExecutable*)),
					 this, SLOT(updateListing(GTestExecutable*)));
	gtest->produceListing();
}

void GTestRunner::updateListing(GTestExecutable* gtest) {
	qDebug() << "Updating listing";
	const int exitCode = gtest->getExitCode();
	qDebug() << "got exit code:" << exitCode;
	QString exePath = gtest->getExecutablePath();
	qDebug() << "got exe path:" << exePath;
	if(exitCode != 0) {
		QMessageBox::critical(this, "Error Retrieving Test Listing",
					QString("Exit code").append(exitCode).append("was returned from the Google Test executable at").append(exePath).append(". Are you sure this is a valid Google Test unit test executable?"));
		//TODO: perform switch statement of process error.
		return;
	}
	QStringList testList = gtest->getListing();
	qDebug() << "Retrieved listing. Size ="<<testList.size();
	QStringList::iterator it = testList.begin();


	TestTreeWidgetItem* testContainer = new TestTreeWidgetItem(&testTree,QStringList() << exePath);
	testContainer->setFlags(Qt::ItemIsTristate | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	testContainer->setCheckState(0, Qt::Checked);
	QVariant var;
	var.setValue<GTestExecutable*>(gtest);
	testContainer->setData(0,Qt::UserRole,var);
	QObject::connect(this, SIGNAL(aboutToRunTests()),
					 gtest, SLOT(resetRunState()));
	QObject::connect(this, SIGNAL(runningTests()),
					 gtest, SLOT(runTest()));
	QSignalMapper* signalMap = new QSignalMapper(&testTree);
	signalMap->setMapping(gtest, testContainer);
	QObject::connect(gtest, SIGNAL(testResultsReady()),
					 signalMap, SLOT(map()));

	TestTreeWidgetItem* topLevelItem = 0;
	TestTreeWidgetItem* newItem = 0;
	GTestSuite* suite = 0;
	GTest* test = 0;
	while(it != testList.end()) {
		qDebug() << *it;
		if(it->endsWith(".")) {
			//drop the '.' and make it a data item
			QString suiteName = it->left(it->size()-1);
			topLevelItem = new TestTreeWidgetItem(testContainer, QStringList()<<suiteName);
			topLevelItem->setFlags(Qt::ItemIsTristate | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			topLevelItem->setCheckState(0, Qt::Checked);
			suite = new GTestSuite(gtest, suiteName);
			var.setValue<GTestSuite*>(suite);
			topLevelItem->setData(0,Qt::UserRole,var);
			gtest->addTest(suite);

			signalMap->setMapping(suite, topLevelItem);
			QObject::connect(suite, SIGNAL(testResultsReady()),
							 signalMap, SLOT(map()));
		}
		else {
			//drop the spaces and make it a data item
			QString testName = it->right(it->size()-2);
			newItem = new TestTreeWidgetItem(topLevelItem, QStringList()<<testName);
			newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			newItem->setCheckState(0,Qt::Checked);
			test = new GTest(suite, testName);
			var.setValue<GTest*>(test);
			newItem->setData(0,Qt::UserRole,var);
			suite->addTest(test);

			signalMap->setMapping(test, newItem);
			QObject::connect(test, SIGNAL(testResultsReady()),
							 signalMap, SLOT(map()));
		}
		++it;
	}
	QObject::connect(signalMap, SIGNAL(mapped(QObject*)),
					 &testTree, SLOT(populateTestResult(QObject*)));
}

void GTestRunner::updateAllListings() {

}

void GTestRunner::runTests() {
	emit aboutToRunTests();
	QTreeWidgetItemIterator it(&testTree, QTreeWidgetItemIterator::NoChildren);
	while(*it) {
		if((*it)->checkState(0) != Qt::Checked)
			continue;
		(*it)->data(0,Qt::UserRole).value<GTest*>()->run();
		it++;
	}
	for(int i=0, j=testTree.topLevelItemCount(); i<j; i++) {
		QTreeWidgetItem* topLevelItem = testTree.topLevelItem(i);
		if(topLevelItem->checkState(0) == Qt::Unchecked)
			continue;
		GTestExecutable* gtest = topLevelItem->data(0,Qt::UserRole).value<GTestExecutable*>();
		if(gtest != 0 && gtest->getState() == GTestExecutable::VALID) {
			//QObject::connect(gtest, SIGNAL(testResultsReady(GTest*)),
			//				 this, SLOT(fillTestResults(GTestExecutable*)));
			gtest->setRunFlag(true);
		}
		else
			QMessageBox::warning(this,"Invalid Google Test",
					"An error has occurred when attempting to run a Google Test.");
	}
	emit runningTests();
}

//TODO::Refactor the three copies of setTestResults to one function
//TODO::The above will require a common base class, or a templated function
void GTestRunner::setTestResult(GTest* /*gtest*/) {
}

/*
 *	This function handles correcting the check states. If a parent is
 *	part checked and all its children are part checked, we transition
 *	to fully checked (previous state must have been unchecked and a click
 *	must have been made on the parent). If any of the children are not
 *	part checked, we know that we did not in fact click on the parent
 *	(and thus we can immediately return), so a part checked parent is fine.
 */
void GTestRunner::treeItemClicked(QTreeWidgetItem* item, int /*column*/) {
	if(item->childCount() == 0 || item->parent() == 0)
		return;
	if(item->checkState(0) ==Qt::PartiallyChecked) {
		for(int i=0,j=item->childCount();i<j;i++)
			if(item->child(i)->checkState(0) & (Qt::Unchecked | Qt::Checked))
				return;
		item->setCheckState(0,Qt::Checked);
	}
}


