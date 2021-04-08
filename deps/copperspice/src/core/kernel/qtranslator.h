/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#include <qobject.h>
#include <qbytearray.h>
#include <qscopedpointer.h>

#ifndef QT_NO_TRANSLATION

class QLocale;
class QTranslatorPrivate;

class Q_CORE_EXPORT QTranslator : public QObject
{
   CORE_CS_OBJECT(QTranslator)

 public:
   explicit QTranslator(QObject *parent = nullptr);

   QTranslator(const QTranslator &) = delete;
   QTranslator &operator=(const QTranslator &) = delete;

   ~QTranslator();

   virtual QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const;
   virtual bool isEmpty() const;

   bool load(const QString &filename, const QString &directory = QString(), const QString &search_delimiters = QString(),
                  const QString &suffix = QString());

   bool load(const QLocale &locale, const QString &filename, const QString &prefix = QString(),
                  const QString &directory = QString(), const QString &suffix = QString());

   bool load(const uchar *data, int len);

 protected:
   QScopedPointer<QTranslatorPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QTranslator)
};

#endif // QT_NO_TRANSLATION

#endif
