/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * TestTreeModel.h - Created on 2010-08-28
 *
 * Copyright (C) 2010 Sandy Chapman
 *
 * This library is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License along with this
 * library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TESTTREEMODEL_H_
#define TESTTREEMODEL_H_

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QSharedPointer>
#include <QVariant>

class TestTreeItem;
class GTest;
class GTestExecutable;

/*! \brief The data model that holds the unit test application hierarchy.
 *
 * The unit test hierarchy is structured as a collection of trees. These
 * are each held as branches of an invisible root node (which conveniently
 * holds the model's header data).
 */
class TestTreeModel : public QAbstractItemModel {

	Q_OBJECT;

private:

	/*! \brief This enum specifies whether a recursive
	 * function should be called recursively up and/or
	 * down the unit test tree hierarchy.
	 */
	enum RECURSE {
		NONE        = 0x0,
		TO_PARENT   = 0x1,
		TO_CHILDREN = 0x2
	};

	QHash<QString, QSharedPointer<GTestExecutable> > testExeHash; //!< A hash of all loaded gtest executables.
	QHash<GTest*, TestTreeItem*> itemTestHash; //!< A hash that relates unit tests to tree items

	template <class T, class U>
	TestTreeItem* createNewTreeItem(T* parent, U* test);
	bool setCheckState(TestTreeItem* item, Qt::CheckState state, int recursionDirection = (TO_PARENT | TO_CHILDREN));

signals:
	void aboutToRunTests();
	void runningTests();
	void resettingRunStates();

private slots:
	void updateListing(GTestExecutable* gtest);
	void updateAllListings();
	void populateTestResult();
	void removeSenderItem();
	void runTests();

public:

	enum ERROR {
		NO_ERROR = 0,
		FILE_NOT_FOUND,
		INSUFFICIENT_PRIVILEGES,
		UNKNOWN
	};

	TestTreeModel(QObject* parent = 0);
	~TestTreeModel();

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QVariant headerData(int secton, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	bool insertItem(TestTreeItem* item, int row, TestTreeItem* parent);
	bool insertItems(const QList<TestTreeItem*> items, int row, TestTreeItem* parent);
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	ERROR addDataSource(const QString filepath);

private:
	TestTreeItem* rootItem; //!< This is the root of the data model.

};

#endif /* TESTTREEMODEL_H_ */
