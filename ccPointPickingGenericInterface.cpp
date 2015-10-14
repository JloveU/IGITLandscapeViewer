//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccPointPickingGenericInterface.h"

//Local
#include "ccGLWindow.h"
#include "mainwindow.h"
#include "db_tree/ccDBRoot.h"

//qCC_db
#include <ccLog.h>
#include <ccPointCloud.h>
#include <cc2DLabel.h>
#include <ccMesh.h>

bool ccPointPickingGenericInterface::linkWith(ccGLWindow* win)
{
	ccGLWindow* oldWin = m_associatedWin;

	if (!ccOverlayDialog::linkWith(win))
		return false;

	//if the dialog is already linked to a window, we must disconnect the 'point picked' signal
	if (oldWin && win != oldWin)
	{
		disconnect(oldWin, SIGNAL(itemPicked(int, unsigned, int, int)), this, SLOT(handlePickedItem(int, unsigned, int, int)));
	}
	//then we can connect the new window 'point picked' signal
	if (m_associatedWin)
	{
		connect(m_associatedWin, SIGNAL(itemPicked(int, unsigned, int, int)), this, SLOT(handlePickedItem(int, unsigned, int, int)));
	}

	return true;
}

bool ccPointPickingGenericInterface::start()
{
	if (!m_associatedWin)
	{
		ccLog::Error("[Point picking] No associated display!");
		return false;
	}

	//activate "point picking mode" in associated GL window
	//m_associatedWin->setPickingMode(ccGLWindow::POINT_PICKING);
    //activate "point or triangle picking mode" in associated GL window
	m_associatedWin->setPickingMode(ccGLWindow::POINT_OR_TRIANGLE_PICKING);
	//the user must not close this window!
	m_associatedWin->setUnclosable(true);
	m_associatedWin->redraw();

	ccOverlayDialog::start();

	return true;
}

void ccPointPickingGenericInterface::stop(bool state)
{
	if (m_associatedWin)
	{
		//deactivate "point picking mode" in all GL windows
		m_associatedWin->setPickingMode(ccGLWindow::DEFAULT_PICKING);
		m_associatedWin->setUnclosable(false);
		m_associatedWin->redraw();
	}

	ccOverlayDialog::stop(state);
}

void ccPointPickingGenericInterface::handlePickedItem(int entityID, unsigned itemIdx, int x, int y)
{
	if (!m_processing)
		return;

	ccHObject* obj = MainWindow::TheInstance()->db()->find(entityID);
	if (!obj)
		return;
	
	if (obj->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		ccPointCloud* cloud = static_cast<ccPointCloud*>(obj);
		if (!cloud)
		{
			assert(false);
			ccLog::Warning("[Item picking] Picked point is not in pickable entities DB?!");
			return;
		}

		const CCVector3* P = cloud->getPoint(itemIdx);
		if (P)
		{
			processPickedPoint(cloud, itemIdx, x, y);
		}
		else
		{
			ccLog::Warning("[Item picking] Invalid point index!");
		}
	}
	else if (obj->isKindOf(CC_TYPES::MESH))
	{
		//NOT HANDLED: 'POINT_PICKING' mode only for now
		//assert(false);
        //ccLog::Warning("[Item picking] Picked entity is mesh(not point)!");
        
        ccMesh* mesh = static_cast<ccMesh*>(obj);
        if (!mesh)
        {
            assert(false);
            ccLog::Warning("[Item picking] Picked point is not in pickable entities DB?!");
            return;
        }

        CCLib::TriangleSummitsIndexes* summitsIndexes = mesh->getTriangleIndexes(itemIdx);
        if (summitsIndexes)
        {
            processPickedTriangle(mesh, itemIdx, x, y);
        }
        else
        {
            ccLog::Warning("[Item picking] Invalid triangle index!");
        }
	}
	else
	{
		//unhandled entity
		//assert(false);
        ccLog::Warning("[Item picking] Picked entity is unknown!");
	}
}
