/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <qtest.h>
#include <QtTest/QSignalSpy>
#include "../../../shared/util.h"
#include "../shared/testhttpserver.h"
#include <math.h>
#include <QFile>
#include <QTextDocument>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeexpression.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <private/qdeclarativetextedit_p.h>
#include <private/qdeclarativetextedit_p_p.h>
#include <QFontMetrics>
#include <QDeclarativeView>
#include <QStyle>
#include <QInputContext>

class tst_qdeclarativetextedit : public QObject

{
    Q_OBJECT
public:
    tst_qdeclarativetextedit();

private slots:
    void text();
    void width();
    void wrap();
    void textFormat();

    // ### these tests may be trivial    
    void hAlign();
    void vAlign();
    void font();
    void color();
    void textMargin();
    void persistentSelection();
    void focusOnPress();
    void selection();
    void mouseSelection_data();
    void mouseSelection();
    void inputMethodHints();

    void cursorDelegate();
    void delegateLoading_data();
    void delegateLoading();
    void navigation();
    void readOnly();
    void openInputPanelOnClick();
    void openInputPanelOnFocus();
    void geometrySignals();
private:
    void simulateKey(QDeclarativeView *, int key);
    QDeclarativeView *createView(const QString &filename);

    QStringList standard;
    QStringList richText;

    QStringList hAlignmentStrings;
    QStringList vAlignmentStrings;

    QList<Qt::Alignment> vAlignments;
    QList<Qt::Alignment> hAlignments;

    QStringList colorStrings;

    QDeclarativeEngine engine;
};

tst_qdeclarativetextedit::tst_qdeclarativetextedit()
{
    standard << "the quick brown fox jumped over the lazy dog"
             << "the quick brown fox\n jumped over the lazy dog";

    richText << "<i>the <b>quick</b> brown <a href=\\\"http://www.google.com\\\">fox</a> jumped over the <b>lazy</b> dog</i>"
             << "<i>the <b>quick</b> brown <a href=\\\"http://www.google.com\\\">fox</a><br>jumped over the <b>lazy</b> dog</i>";

    hAlignmentStrings << "AlignLeft"
                      << "AlignRight"
                      << "AlignHCenter";

    vAlignmentStrings << "AlignTop"
                      << "AlignBottom"
                      << "AlignVCenter";

    hAlignments << Qt::AlignLeft
                << Qt::AlignRight
                << Qt::AlignHCenter;

    vAlignments << Qt::AlignTop
                << Qt::AlignBottom
                << Qt::AlignVCenter;

    colorStrings << "aliceblue"
                 << "antiquewhite"
                 << "aqua"
                 << "darkkhaki"
                 << "darkolivegreen"
                 << "dimgray"
                 << "palevioletred"
                 << "lightsteelblue"
                 << "#000000"
                 << "#AAAAAA"
                 << "#FFFFFF"
                 << "#2AC05F";
                 //
                 // need a different test to do alpha channel test
                 // << "#AA0011DD"
                 // << "#00F16B11";
                 // 
}

void tst_qdeclarativetextedit::text()
{
    {
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData("import Qt 4.7\nTextEdit {  text: \"\"  }", QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->text(), QString(""));
    }

    for (int i = 0; i < standard.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit { text: \"" + standard.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->text(), standard.at(i));
    }

    for (int i = 0; i < richText.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit { text: \"" + richText.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QString actual = textEditObject->text();
        QString expected = richText.at(i);
        actual.replace(QRegExp(".*<body[^>]*>"),"");
        actual.replace(QRegExp("(<[^>]*>)+"),"<>");
        expected.replace(QRegExp("(<[^>]*>)+"),"<>");
        QCOMPARE(actual.simplified(),expected.simplified());
    }
}

void tst_qdeclarativetextedit::width()
{
    // uses Font metrics to find the width for standard and document to find the width for rich
    {
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData("import Qt 4.7\nTextEdit {  text: \"\" }", QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), 1.);//+1 for cursor
    }

    for (int i = 0; i < standard.size(); i++)
    {
        QFont f;
        QFontMetricsF fm(f);
        qreal metricWidth = fm.size(Qt::TextExpandTabs && Qt::TextShowMnemonic, standard.at(i)).width();
        metricWidth = ceil(metricWidth);

        QString componentStr = "import Qt 4.7\nTextEdit { text: \"" + standard.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), qreal(metricWidth + 1 + 3));//+3 is the current way of accounting for space between cursor and last character.
    }

    for (int i = 0; i < richText.size(); i++)
    {
        QTextDocument document;
        document.setHtml(richText.at(i));
        document.setDocumentMargin(0);

        int documentWidth = ceil(document.idealWidth());

        QString componentStr = "import Qt 4.7\nTextEdit { text: \"" + richText.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), qreal(documentWidth + 1 + 3));
    }
}

void tst_qdeclarativetextedit::wrap()
{
    // for specified width and wrap set true
    {
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData("import Qt 4.7\nTextEdit {  text: \"\"; wrapMode: TextEdit.WordWrap; width: 300 }", QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), 300.);
    }

    for (int i = 0; i < standard.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  wrapMode: TextEdit.WordWrap; width: 300; text: \"" + standard.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), 300.);
    }

    for (int i = 0; i < richText.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  wrapMode: TextEdit.WordWrap; width: 300; text: \"" + richText.at(i) + "\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->width(), 300.);
    }

}

void tst_qdeclarativetextedit::textFormat()
{
    {
        QDeclarativeComponent textComponent(&engine);
        textComponent.setData("import Qt 4.7\nTextEdit { text: \"Hello\"; textFormat: Text.RichText }", QUrl::fromLocalFile(""));
        QDeclarativeTextEdit *textObject = qobject_cast<QDeclarativeTextEdit*>(textComponent.create());

        QVERIFY(textObject != 0);
        QVERIFY(textObject->textFormat() == QDeclarativeTextEdit::RichText);
    }
    {
        QDeclarativeComponent textComponent(&engine);
        textComponent.setData("import Qt 4.7\nTextEdit { text: \"<b>Hello</b>\"; textFormat: Text.PlainText }", QUrl::fromLocalFile(""));
        QDeclarativeTextEdit *textObject = qobject_cast<QDeclarativeTextEdit*>(textComponent.create());

        QVERIFY(textObject != 0);
        QVERIFY(textObject->textFormat() == QDeclarativeTextEdit::PlainText);
    }
}

//the alignment tests may be trivial o.oa
void tst_qdeclarativetextedit::hAlign()
{
    //test one align each, and then test if two align fails.

    for (int i = 0; i < standard.size(); i++)
    {
        for (int j=0; j < hAlignmentStrings.size(); j++)
        {
            QString componentStr = "import Qt 4.7\nTextEdit {  horizontalAlignment: \"" + hAlignmentStrings.at(j) + "\"; text: \"" + standard.at(i) + "\" }";
            QDeclarativeComponent texteditComponent(&engine);
            texteditComponent.setData(componentStr.toLatin1(), QUrl());
            QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

            QVERIFY(textEditObject != 0);
            QCOMPARE((int)textEditObject->hAlign(), (int)hAlignments.at(j));
        }
    }

    for (int i = 0; i < richText.size(); i++)
    {
        for (int j=0; j < hAlignmentStrings.size(); j++)
        {
            QString componentStr = "import Qt 4.7\nTextEdit {  horizontalAlignment: \"" + hAlignmentStrings.at(j) + "\"; text: \"" + richText.at(i) + "\" }";
            QDeclarativeComponent texteditComponent(&engine);
            texteditComponent.setData(componentStr.toLatin1(), QUrl());
            QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

            QVERIFY(textEditObject != 0);
            QCOMPARE((int)textEditObject->hAlign(), (int)hAlignments.at(j));
        }
    }

}

void tst_qdeclarativetextedit::vAlign()
{
    //test one align each, and then test if two align fails.

    for (int i = 0; i < standard.size(); i++)
    {
        for (int j=0; j < vAlignmentStrings.size(); j++)
        {
            QString componentStr = "import Qt 4.7\nTextEdit {  verticalAlignment: \"" + vAlignmentStrings.at(j) + "\"; text: \"" + standard.at(i) + "\" }";
            QDeclarativeComponent texteditComponent(&engine);
            texteditComponent.setData(componentStr.toLatin1(), QUrl());
            QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

            QVERIFY(textEditObject != 0);
            QCOMPARE((int)textEditObject->vAlign(), (int)vAlignments.at(j));
        }
    }

    for (int i = 0; i < richText.size(); i++)
    {
        for (int j=0; j < vAlignmentStrings.size(); j++)
        {
            QString componentStr = "import Qt 4.7\nTextEdit {  verticalAlignment: \"" + vAlignmentStrings.at(j) + "\"; text: \"" + richText.at(i) + "\" }";
            QDeclarativeComponent texteditComponent(&engine);
            texteditComponent.setData(componentStr.toLatin1(), QUrl());
            QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

            QVERIFY(textEditObject != 0);
            QCOMPARE((int)textEditObject->vAlign(), (int)vAlignments.at(j));
        }
    }

}

void tst_qdeclarativetextedit::font()
{
    //test size, then bold, then italic, then family
    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  font.pointSize: 40; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->font().pointSize(), 40);
        QCOMPARE(textEditObject->font().bold(), false);
        QCOMPARE(textEditObject->font().italic(), false);
    }

    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  font.bold: true; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->font().bold(), true);
        QCOMPARE(textEditObject->font().italic(), false);
    }

    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  font.italic: true; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->font().italic(), true);
        QCOMPARE(textEditObject->font().bold(), false);
    }
 
    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  font.family: \"Helvetica\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->font().family(), QString("Helvetica"));
        QCOMPARE(textEditObject->font().bold(), false);
        QCOMPARE(textEditObject->font().italic(), false);
    }

    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  font.family: \"\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->font().family(), QString(""));
    }
}

void tst_qdeclarativetextedit::color()
{
    //test normal
    for (int i = 0; i < colorStrings.size(); i++)
    { 
        QString componentStr = "import Qt 4.7\nTextEdit {  color: \"" + colorStrings.at(i) + "\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        //qDebug() << "textEditObject: " << textEditObject->color() << "vs. " << QColor(colorStrings.at(i));
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->color(), QColor(colorStrings.at(i)));
    }

    //test selection
    for (int i = 0; i < colorStrings.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  selectionColor: \"" + colorStrings.at(i) + "\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->selectionColor(), QColor(colorStrings.at(i)));
    }

    //test selected text
    for (int i = 0; i < colorStrings.size(); i++)
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  selectedTextColor: \"" + colorStrings.at(i) + "\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->selectedTextColor(), QColor(colorStrings.at(i)));
    }

    {
        QString colorStr = "#AA001234";
        QColor testColor("#001234");
        testColor.setAlpha(170);

        QString componentStr = "import Qt 4.7\nTextEdit {  color: \"" + colorStr + "\"; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());

        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->color(), testColor);
    }
}

void tst_qdeclarativetextedit::textMargin()
{
    for(qreal i=0; i<=10; i+=0.3){
        QString componentStr = "import Qt 4.7\nTextEdit {  textMargin: " + QString::number(i) + "; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->textMargin(), i);
    }
}

void tst_qdeclarativetextedit::persistentSelection()
{
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  persistentSelection: true; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->persistentSelection(), true);
    }

    {
        QString componentStr = "import Qt 4.7\nTextEdit {  persistentSelection: false; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->persistentSelection(), false);
    }
}

void tst_qdeclarativetextedit::focusOnPress()
{
    {
        QString componentStr = "import Qt 4.7\nTextEdit {  focusOnPress: true; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->focusOnPress(), true);
    }

    {
        QString componentStr = "import Qt 4.7\nTextEdit {  focusOnPress: false; text: \"Hello World\" }";
        QDeclarativeComponent texteditComponent(&engine);
        texteditComponent.setData(componentStr.toLatin1(), QUrl());
        QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
        QVERIFY(textEditObject != 0);
        QCOMPARE(textEditObject->focusOnPress(), false);
    }
}

void tst_qdeclarativetextedit::selection()
{
    QString testStr = standard[0];//TODO: What should happen for multiline/rich text?
    QString componentStr = "import Qt 4.7\nTextEdit {  text: \""+ testStr +"\"; }";
    QDeclarativeComponent texteditComponent(&engine);
    texteditComponent.setData(componentStr.toLatin1(), QUrl());
    QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit*>(texteditComponent.create());
    QVERIFY(textEditObject != 0);


    //Test selection follows cursor
    for(int i=0; i<= testStr.size(); i++) {
        textEditObject->setCursorPosition(i);
        QCOMPARE(textEditObject->cursorPosition(), i);
        QCOMPARE(textEditObject->selectionStart(), i);
        QCOMPARE(textEditObject->selectionEnd(), i);
        QVERIFY(textEditObject->selectedText().isNull());
    }

    textEditObject->setCursorPosition(0);
    QVERIFY(textEditObject->cursorPosition() == 0);
    QVERIFY(textEditObject->selectionStart() == 0);
    QVERIFY(textEditObject->selectionEnd() == 0);
    QVERIFY(textEditObject->selectedText().isNull());

    //Test selection
    for(int i=0; i<= testStr.size(); i++) {
        textEditObject->select(0,i);
        QCOMPARE(testStr.mid(0,i), textEditObject->selectedText());
    }
    for(int i=0; i<= testStr.size(); i++) {
        textEditObject->select(i,testStr.size());
        QCOMPARE(testStr.mid(i,testStr.size()-i), textEditObject->selectedText());
    }

    textEditObject->setCursorPosition(0);
    QVERIFY(textEditObject->cursorPosition() == 0);
    QVERIFY(textEditObject->selectionStart() == 0);
    QVERIFY(textEditObject->selectionEnd() == 0);
    QVERIFY(textEditObject->selectedText().isNull());

    //Test Error Ignoring behaviour
    textEditObject->setCursorPosition(0);
    QVERIFY(textEditObject->selectedText().isNull());
    textEditObject->select(-10,0);
    QVERIFY(textEditObject->selectedText().isNull());
    textEditObject->select(100,101);
    QVERIFY(textEditObject->selectedText().isNull());
    textEditObject->select(0,-10);
    QVERIFY(textEditObject->selectedText().isNull());
    textEditObject->select(0,100);
    QVERIFY(textEditObject->selectedText().isNull());
    textEditObject->select(0,10);
    QVERIFY(textEditObject->selectedText().size() == 10);
    textEditObject->select(-10,0);
    QVERIFY(textEditObject->selectedText().size() == 10);
    textEditObject->select(100,101);
    QVERIFY(textEditObject->selectedText().size() == 10);
    textEditObject->select(0,-10);
    QVERIFY(textEditObject->selectedText().size() == 10);
    textEditObject->select(0,100);
    QVERIFY(textEditObject->selectedText().size() == 10);
}

void tst_qdeclarativetextedit::mouseSelection_data()
{
    QTest::addColumn<QString>("qmlfile");
    QTest::addColumn<bool>("expectSelection");

    // import installed
    QTest::newRow("on") << SRCDIR "/data/mouseselection_true.qml" << true;
    QTest::newRow("off") << SRCDIR "/data/mouseselection_false.qml" << false;
    QTest::newRow("default") << SRCDIR "/data/mouseselection_default.qml" << false;
}

void tst_qdeclarativetextedit::mouseSelection()
{
    QFETCH(QString, qmlfile);
    QFETCH(bool, expectSelection);

    QDeclarativeView *canvas = createView(qmlfile);

    canvas->show();
    QApplication::setActiveWindow(canvas);
    QTest::qWaitForWindowShown(canvas);
    QTRY_COMPARE(QApplication::activeWindow(), static_cast<QWidget *>(canvas));

    QVERIFY(canvas->rootObject() != 0);
    QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit *>(canvas->rootObject());
    QVERIFY(textEditObject != 0);

    // press-and-drag-and-release from x1 to x2
    int x1 = 10;
    int x2 = 70;
    int y = textEditObject->height()/2;
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, 0, canvas->mapFromScene(QPoint(x1,y)));
    //QTest::mouseMove(canvas->viewport(), canvas->mapFromScene(QPoint(x2,y))); // doesn't work
    QMouseEvent mv(QEvent::MouseMove, canvas->mapFromScene(QPoint(x2,y)), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(canvas->viewport(), &mv);
    QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, 0, canvas->mapFromScene(QPoint(x2,y)));
    QString str = textEditObject->selectedText();
    if (expectSelection)
        QVERIFY(str.length() > 3); // don't reallly care *what* was selected (and it's too sensitive to platform)
    else
        QVERIFY(str.isEmpty());
}

void tst_qdeclarativetextedit::inputMethodHints()
{
    QDeclarativeView *canvas = createView(SRCDIR "/data/inputmethodhints.qml");
    canvas->show();
    canvas->setFocus();

    QVERIFY(canvas->rootObject() != 0);
    QDeclarativeTextEdit *textEditObject = qobject_cast<QDeclarativeTextEdit *>(canvas->rootObject());
    QVERIFY(textEditObject != 0);
    QVERIFY(textEditObject->inputMethodHints() & Qt::ImhNoPredictiveText);
    textEditObject->setInputMethodHints(Qt::ImhUppercaseOnly);
    QVERIFY(textEditObject->inputMethodHints() & Qt::ImhUppercaseOnly);
}

void tst_qdeclarativetextedit::cursorDelegate()
{
    QDeclarativeView* view = createView(SRCDIR "/data/cursorTest.qml");
    view->show();
    view->setFocus();
    QDeclarativeTextEdit *textEditObject = view->rootObject()->findChild<QDeclarativeTextEdit*>("textEditObject");
    QVERIFY(textEditObject != 0);
    QVERIFY(textEditObject->findChild<QDeclarativeItem*>("cursorInstance"));
    //Test Delegate gets created
    textEditObject->setFocus(true);
    QDeclarativeItem* delegateObject = textEditObject->findChild<QDeclarativeItem*>("cursorInstance");
    QVERIFY(delegateObject);
    //Test Delegate gets moved
    for(int i=0; i<= textEditObject->text().length(); i++){
        textEditObject->setCursorPosition(i);
        QCOMPARE(textEditObject->cursorRectangle().x(), qRound(delegateObject->x()));
        QCOMPARE(textEditObject->cursorRectangle().y(), qRound(delegateObject->y()));
    }
    textEditObject->setCursorPosition(0);
    QCOMPARE(textEditObject->cursorRectangle().x(), qRound(delegateObject->x()));
    QCOMPARE(textEditObject->cursorRectangle().y(), qRound(delegateObject->y()));
    //Test Delegate gets deleted
    textEditObject->setCursorDelegate(0);
    QVERIFY(!textEditObject->findChild<QDeclarativeItem*>("cursorInstance"));
}

void tst_qdeclarativetextedit::delegateLoading_data()
{
    QTest::addColumn<QString>("qmlfile");
    QTest::addColumn<QString>("error");

    // import installed
    QTest::newRow("pass") << "cursorHttpTestPass.qml" << "";
    QTest::newRow("fail1") << "cursorHttpTestFail1.qml" << "<Unknown File>: Network error for URL http://localhost:42332/FailItem.qml ";
    QTest::newRow("fail2") << "cursorHttpTestFail2.qml" << "http://localhost:42332/ErrItem.qml:4:5: Fungus is not a type ";
}

void tst_qdeclarativetextedit::delegateLoading()
{
    QFETCH(QString, qmlfile);
    QFETCH(QString, error);

    TestHTTPServer server(42332);
    server.serveDirectory(SRCDIR "/data/httpfail", TestHTTPServer::Disconnect);
    server.serveDirectory(SRCDIR "/data/httpslow", TestHTTPServer::Delay);
    server.serveDirectory(SRCDIR "/data/http");

    QDeclarativeView* view = new QDeclarativeView(0);

    view->setSource(QUrl(QLatin1String("http://localhost:42332/") + qmlfile));
    view->show();
    view->setFocus();

    if (!error.isEmpty()) {
        QTest::ignoreMessage(QtWarningMsg, error.toUtf8());
        QTRY_VERIFY(view->status()==QDeclarativeView::Error);
        QTRY_VERIFY(!view->rootObject()); // there is fail item inside this test
    } else {
        QTRY_VERIFY(view->rootObject());//Wait for loading to finish.
        QDeclarativeTextEdit *textEditObject = view->rootObject()->findChild<QDeclarativeTextEdit*>("textEditObject");
        //    view->rootObject()->dumpObjectTree();
        QVERIFY(textEditObject != 0);
        textEditObject->setFocus(true);
        QDeclarativeItem *delegate;
        delegate = view->rootObject()->findChild<QDeclarativeItem*>("delegateOkay");
        QVERIFY(delegate);
        delegate = view->rootObject()->findChild<QDeclarativeItem*>("delegateSlow");
        QVERIFY(delegate);

        delete delegate;
    }


    //A test should be added here with a component which is ready but component.create() returns null
    //Not sure how to accomplish this with QDeclarativeTextEdits cursor delegate
    //###This was only needed for code coverage, and could be a case of overzealous defensive programming
    //delegate = view->rootObject()->findChild<QDeclarativeItem*>("delegateErrorB");
    //QVERIFY(!delegate);

    delete view;
}

/*
TextEdit element should only handle left/right keys until the cursor reaches
the extent of the text, then they should ignore the keys.
*/
void tst_qdeclarativetextedit::navigation()
{
    QDeclarativeView *canvas = createView(SRCDIR "/data/navigation.qml");
    canvas->show();
    canvas->setFocus();

    QVERIFY(canvas->rootObject() != 0);

    QDeclarativeItem *input = qobject_cast<QDeclarativeItem *>(qvariant_cast<QObject *>(canvas->rootObject()->property("myInput")));

    QVERIFY(input != 0);
    QTRY_VERIFY(input->hasFocus() == true);
    simulateKey(canvas, Qt::Key_Left);
    QVERIFY(input->hasFocus() == false);
    simulateKey(canvas, Qt::Key_Right);
    QVERIFY(input->hasFocus() == true);
    simulateKey(canvas, Qt::Key_Right);
    QVERIFY(input->hasFocus() == false);
    simulateKey(canvas, Qt::Key_Left);
    QVERIFY(input->hasFocus() == true);
}

void tst_qdeclarativetextedit::readOnly()
{
    QDeclarativeView *canvas = createView(SRCDIR "/data/readOnly.qml");
    canvas->show();
    canvas->setFocus();

    QVERIFY(canvas->rootObject() != 0);

    QDeclarativeTextEdit *edit = qobject_cast<QDeclarativeTextEdit *>(qvariant_cast<QObject *>(canvas->rootObject()->property("myInput")));

    QVERIFY(edit != 0);
    QTRY_VERIFY(edit->hasFocus() == true);
    QVERIFY(edit->isReadOnly() == true);
    QString initial = edit->text();
    for(int k=Qt::Key_0; k<=Qt::Key_Z; k++)
        simulateKey(canvas, k);
    simulateKey(canvas, Qt::Key_Return);
    simulateKey(canvas, Qt::Key_Space);
    simulateKey(canvas, Qt::Key_Escape);
    QCOMPARE(edit->text(), initial);
}

void tst_qdeclarativetextedit::simulateKey(QDeclarativeView *view, int key)
{
    QKeyEvent press(QKeyEvent::KeyPress, key, 0);
    QKeyEvent release(QKeyEvent::KeyRelease, key, 0);

    QApplication::sendEvent(view, &press);
    QApplication::sendEvent(view, &release);
}

QDeclarativeView *tst_qdeclarativetextedit::createView(const QString &filename)
{
    QDeclarativeView *canvas = new QDeclarativeView(0);

    canvas->setSource(QUrl::fromLocalFile(filename));
    return canvas;
}

class MyInputContext : public QInputContext
{
public:
    MyInputContext() : openInputPanelReceived(false), closeInputPanelReceived(false) {}
    ~MyInputContext() {}

    QString identifierName() { return QString(); }
    QString language() { return QString(); }

    void reset() {}

    bool isComposing() const { return false; }

    bool filterEvent( const QEvent *event )
    {
        if (event->type() == QEvent::RequestSoftwareInputPanel)
            openInputPanelReceived = true;
        if (event->type() == QEvent::CloseSoftwareInputPanel)
            closeInputPanelReceived = true;
        return QInputContext::filterEvent(event);
    }
    bool openInputPanelReceived;
    bool closeInputPanelReceived;
};

void tst_qdeclarativetextedit::openInputPanelOnClick()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    MyInputContext ic;
    view.setInputContext(&ic);
    QDeclarativeTextEdit edit;
    QSignalSpy focusOnPressSpy(&edit, SIGNAL(focusOnPressChanged(bool)));
    edit.setText("Hello world");
    edit.setPos(0, 0);
    scene.addItem(&edit);
    view.show();
    qApp->setAutoSipEnabled(true);
    QApplication::setActiveWindow(&view);
    QTest::qWaitForWindowShown(&view);
    QTRY_COMPARE(QApplication::activeWindow(), static_cast<QWidget *>(&view));

    QDeclarativeItemPrivate* pri = QDeclarativeItemPrivate::get(&edit);
    QDeclarativeTextEditPrivate *editPrivate = static_cast<QDeclarativeTextEditPrivate*>(pri);

    // input panel on click
    editPrivate->showInputPanelOnFocus = false;

    QStyle::RequestSoftwareInputPanel behavior = QStyle::RequestSoftwareInputPanel(
            view.style()->styleHint(QStyle::SH_RequestSoftwareInputPanel));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    QApplication::processEvents();
    if (behavior == QStyle::RSIP_OnMouseClickAndAlreadyFocused) {
        QCOMPARE(ic.openInputPanelReceived, false);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
        QApplication::processEvents();
        QCOMPARE(ic.openInputPanelReceived, true);
    } else if (behavior == QStyle::RSIP_OnMouseClick) {
        QCOMPARE(ic.openInputPanelReceived, true);
    }
    ic.openInputPanelReceived = false;

    // focus should not cause input panels to open or close
    edit.setFocus(false);
    edit.setFocus(true);
    edit.setFocus(false);
    edit.setFocus(true);
    edit.setFocus(false);
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);
}

void tst_qdeclarativetextedit::openInputPanelOnFocus()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    MyInputContext ic;
    view.setInputContext(&ic);
    QDeclarativeTextEdit edit;
    QSignalSpy focusOnPressSpy(&edit, SIGNAL(focusOnPressChanged(bool)));
    edit.setText("Hello world");
    edit.setPos(0, 0);
    scene.addItem(&edit);
    view.show();
    qApp->setAutoSipEnabled(true);
    QApplication::setActiveWindow(&view);
    QTest::qWaitForWindowShown(&view);
    QTRY_COMPARE(QApplication::activeWindow(), static_cast<QWidget *>(&view));

    QDeclarativeItemPrivate* pri = QDeclarativeItemPrivate::get(&edit);
    QDeclarativeTextEditPrivate *editPrivate = static_cast<QDeclarativeTextEditPrivate*>(pri);
    editPrivate->showInputPanelOnFocus = true;

    QVERIFY(edit.focusOnPress());
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);

    // focus on press, input panel on focus
    QTest::mousePress(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    QApplication::processEvents();
    QVERIFY(edit.hasFocus());
    QCOMPARE(ic.openInputPanelReceived, true);
    QCOMPARE(ic.closeInputPanelReceived, false);
    ic.openInputPanelReceived = false;

    // no events on release
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);
    ic.openInputPanelReceived = false;

    // Even with focus already gained, user needs
    // to be able to open panel by pressing on the editor
    QTest::mousePress(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    QApplication::processEvents();
    QCOMPARE(ic.openInputPanelReceived, true);
    QCOMPARE(ic.closeInputPanelReceived, false);
    ic.openInputPanelReceived = false;

    // input panel closed on focus lost
    edit.setFocus(false);
    QApplication::processEvents();
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, true);
    ic.closeInputPanelReceived = false;

    // no automatic input panel events if focusOnPress is false
    edit.setFocusOnPress(false);
    QCOMPARE(focusOnPressSpy.count(),1);
    QTest::mousePress(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(edit.scenePos()));
    edit.setFocus(false);
    edit.setFocus(true);
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);

    edit.setFocusOnPress(false);
    QCOMPARE(focusOnPressSpy.count(),1);

    // one show input panel event when openSoftwareInputPanel is called
    edit.openSoftwareInputPanel();
    QCOMPARE(ic.openInputPanelReceived, true);
    QCOMPARE(ic.closeInputPanelReceived, false);
    ic.openInputPanelReceived = false;

    // one close input panel event when closeSoftwareInputPanel is called
    edit.closeSoftwareInputPanel();
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, true);
    ic.openInputPanelReceived = false;

    // set focusOnPress back to true
    edit.setFocusOnPress(true);
    QCOMPARE(focusOnPressSpy.count(),2);
    edit.setFocus(false);
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, true);
    ic.closeInputPanelReceived = false;

    edit.setFocusOnPress(true);
    QCOMPARE(focusOnPressSpy.count(),2);

    // active window focus reason should not cause input panel to open
    QGraphicsObject * editObject = qobject_cast<QGraphicsObject*>(&edit);
    editObject->setFocus(Qt::ActiveWindowFocusReason);
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);

    // and input panel should not open if focus has already been set
    edit.setFocus(true);
    QCOMPARE(ic.openInputPanelReceived, false);
    QCOMPARE(ic.closeInputPanelReceived, false);
}

void tst_qdeclarativetextedit::geometrySignals()
{
    QDeclarativeComponent component(&engine, SRCDIR "/data/geometrySignals.qml");
    QObject *o = component.create();
    QVERIFY(o);
    QCOMPARE(o->property("bindingWidth").toInt(), 400);
    QCOMPARE(o->property("bindingHeight").toInt(), 500);
    delete o;
}

QTEST_MAIN(tst_qdeclarativetextedit)

#include "tst_qdeclarativetextedit.moc"