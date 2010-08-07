/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * GTestExecutable.h - Created on 2010-07-23                                                           *
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

#ifndef GTESTEXECUTABLE_H_
#define GTESTEXECUTABLE_H_

#include <QBuffer>
#include <QObject>
#include <QProcess>
#include <QMetaType>
#include <QMutex>

#include "GTestExecutableResults.h"
#include "GTestSuite.h"

class GTestExecutable : public GTestSuite {

Q_OBJECT

public:
	enum STATE {
		VALID = 0x0,
		FILE_NOT_FOUND = 0x1,
		INSUFFICIENT_PRIVILEGES = 0x2
	};

private:
	STATE state;
	QMutex processLock;
	QMutex outputLock;
	QProcess *gtest;
	QBuffer standardOutput;
	QBuffer standardError;
	QStringList listing;
	QStringList testsToRun;
	bool runOnSignal;

	QProcess::ProcessError error;
	QProcess::ExitStatus exitStatus;
	int exitCode;

	void runExecutable(QStringList args);
	void readExecutableOutput(QBuffer& standardChannel);

signals:
	void listingReady(GTestExecutable* sender);

public slots:
	void standardErrorAvailable();
	void standardOutputAvailable();
	void executableFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void runTest();
	void parseListing(int exitCode, QProcess::ExitStatus exitStatus);
	void parseTestResults(int exitCode, QProcess::ExitStatus exitStatus);
	void resetRunState();

public:

	GTestExecutable(QObject* parent = 0, QString executablePath = QString());
	GTestExecutable(const GTestExecutable& other);
	virtual ~GTestExecutable();

//GETS:
	QProcess::ProcessError getError() const;
	QString getExecutablePath() const;
	int getExitCode() const;
	QProcess::ExitStatus getExitStatus() const;
	QStringList getListing() const;
	STATE getState();
	void setRunFlag(bool runOnSignal);

//SETS:
	void setExecutablePath(QString executablePath);

//METHODS:
	void produceListing();

};

Q_DECLARE_METATYPE(GTestExecutable*);

inline QStringList GTestExecutable::getListing() const { return listing; }

inline QString GTestExecutable::getExecutablePath() const { return name; }

inline QProcess::ProcessError GTestExecutable::getError() const { return error; }

inline QProcess::ExitStatus GTestExecutable::getExitStatus() const { return exitStatus; }

inline int GTestExecutable::getExitCode() const { return exitCode; }

inline void GTestExecutable::setExecutablePath(QString executablePath) { this->name = executablePath; }

inline void GTestExecutable::setRunFlag(bool runOnSignal) { this->runOnSignal = runOnSignal; }

inline void GTestExecutable::resetRunState() { setRunFlag(false); }

#endif /* GTESTEXECUTABLE_H_ */
