/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Changes cba button's label to image.
*
*/

#ifndef EIKSOFTKEYIMAGE_H
#define EIKSOFTKEYIMAGE_H

// FORWARD DECLARATIONS
class CEikButtonGroupContainer;

// CLASS DECLARATION

/**
*  Changes cba button's label to image.
*
*  @lib EIKCOCTL
*  @since 2.0
*/
class EikSoftkeyImage
    {
    public:

        /**
        * Set image to cba button by replacing label
        * @since 2.0
        * @param aButtonGroupContainer Button container
        * @param aImage Image to button,
        *               Takes Images ownership
        * @param aLeft  Boolean: left or right button.
        *                        If true, then change left,
        *                        if false, change right
        */
        IMPORT_C static void SetImage(CEikButtonGroupContainer* aButtonGroupContainer, CEikImage& aImage, TBool aLeft);

        /**
        * Change to cba button image back to label
        * @since 2.0
        * @param aButtonGroupContainer Button container
        * @param aLeft Boolean: left or right button.
        *                        If true, then change left,
        *                        if false, change right
        */
        IMPORT_C static void SetLabel(CEikButtonGroupContainer* aButtonGroupContainer, TBool aLeft);

    private:

        /**
        * C++ default constructor.
        */
        EikSoftkeyImage() {};


    };

#endif      // EIKSOFTKEYIMAGE_H

// End of File
