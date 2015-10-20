#include "MarkedLine.h"
#include "ccSphere.h"
#include "ccGenericGLDisplay.h"

MarkedLine::MarkedLine()
    : MarkedObject()
    , mMarkedName(ccObject::getName())
    , mShortestPathComputer(NULL)
    , mLineLength(0.0)
    , mCurrentPositionInHistory(-1)
{
}

MarkedLine::MarkedLine(QString name)
    : MarkedObject(name)
    , mMarkedName(name)
    , mShortestPathComputer(NULL)
    , mLineLength(0.0)
    , mCurrentPositionInHistory(-1)
{
}

bool MarkedLine::addPoint(ccMesh* mesh, unsigned pointIndex)
{
    assert(mesh);
    assert(mShortestPathComputer);
    assert(mShortestPathComputer->getAssociatedMesh() == mesh);

    ccGenericPointCloud *cloud = mesh->getAssociatedCloud();
    assert(cloud && cloud->size() > pointIndex);

    //计算最短路径
    QList<unsigned> path;
    float pathLength;
    if (size() != 0)
    {
        assert(cloud == getPoint(size() - 1).cloud);
        if (pointIndex == getPoint(size() - 1).index) //如果本次要添加的点和上次添加的点相同，则不做任何操作
        {
            return true;
        }
        else
        {
            path = mShortestPathComputer->getShortestPathVertices(getPoint(size() - 1).index, pointIndex, &pathLength);
            mLineLength += pathLength;
        }
    }
    const unsigned pathSize = path.size();

    //历史纪录
    if (!mHistory.isEmpty() && mCurrentPositionInHistory < mHistory.size() - 1) //舍弃历史纪录中被撤销的操作
    {
        mHistory.remove(mCurrentPositionInHistory + 1, mHistory.size() - mCurrentPositionInHistory- 1);
    }
    if (!mHistory.isEmpty())
    {
        mHistory.insert(mHistory.end(), mHistory.back()); //复制最后一个状态
    }
    else
    {
        mHistory.resize(1);
    }
    for (int i = 0; i < pathSize; i++) //在复制得到的状态上增加本次要添加的点
    {
        mHistory.back().points.push_back(PickedPoint(cloud, path.at(i)));
    }
    mHistory.back().points.push_back(PickedPoint(cloud, pointIndex));
    mHistory.back().lineLength += pathLength;
    mCurrentPositionInHistory = mHistory.size() - 1;

    //更新m_points和mLineLength
    m_points = mHistory.back().points;
    mLineLength = mHistory.back().lineLength;

    updateName();

    //we want to be notified whenever an associated cloud is deleted (in which case
    //we'll automatically clear the label)
    cloud->addDependency(this,DP_NOTIFY_OTHER_ON_DELETE);
    //we must also warn the cloud whenever we delete this label
    //addDependency(cloud,DP_NOTIFY_OTHER_ON_DELETE); //DGM: automatically done by the previous call to addDependency!

    return true;
}

void MarkedLine::clear()
{
    m_points.clear();
}

//copied from cc2DLabel.cpp
//unit point marker
static QSharedPointer<ccSphere> c_unitPointMarker(0);

void MarkedLine::drawMeOnly3D(CC_DRAW_CONTEXT& context)
{
    assert(!m_points.empty());

	//standard case: list names pushing
	bool pushName = MACRO_DrawEntityNames(context);
	if (pushName)
	{
		//not particularily fast
		if (MACRO_DrawFastNamesOnly(context))
			return;
		glPushName(getUniqueIDForDisplay());
	}

    ////画控制点之间的连线（直线）
	const float c_sizeFactor = 10.0f;
	bool loop = false;
    size_t count = m_points.size();
    //segment width
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(c_sizeFactor);
    //we draw the segments
    ccGL::Color3v(ccColor::Rgba(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha()).rgba);
    glBegin(GL_LINES);
    for (unsigned i=0; i<count; i++)
    {
        if (i+1<count || loop)
        {
            ccGL::Vertex3v(m_points[i].cloud->getPoint(m_points[i].index)->u);
            ccGL::Vertex3v(m_points[(i+1)%count].cloud->getPoint(m_points[(i+1)%count].index)->u);
        }
    }
    glEnd();
    glPopAttrib();

    //显示小球标记
    //display point marker as spheres
	{
		if (!c_unitPointMarker)
		{
			c_unitPointMarker = QSharedPointer<ccSphere>(new ccSphere(1.0f,0,"PointMarker",12));
            c_unitPointMarker->showColors(true);
			c_unitPointMarker->setVisible(true);
			c_unitPointMarker->setEnabled(true);
		}
	
		//build-up point maker own 'context'
		CC_DRAW_CONTEXT markerContext = context;
		markerContext.flags &= (~CC_DRAW_ENTITY_NAMES); //we must remove the 'push name flag' so that the sphere doesn't push its own!
		markerContext._win = 0;

        //始终显示同一颜色，不管是否选中状态
        c_unitPointMarker->setTempColor(ccColor::Rgb(mColor.red(), mColor.green(), mColor.blue()));

        for (unsigned i = 0; i < count; i += (count == 1 ? 1 : count - 1)) //只显示首尾端点处的小球
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            const CCVector3* P = m_points[i].cloud->getPoint(m_points[i].index);
            ccGL::Translate(P->x,P->y,P->z);
            glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
            c_unitPointMarker->draw(markerContext);
            glPopMatrix();
        }
	}

	if (m_dispIn3D && !pushName) //no need to display label in point picking mode
	{
		//QFont font(context._win->getTextDisplayFont()); //takes rendering zoom into account!
		//font.setPointSize(font.pointSize()+2);
		//font.setBold(true);
		static const QChar ABC[3] = {'A','B','C'};

		//draw their name
		glPushAttrib(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		for (unsigned j=0; j<count; j++)
		{
			const CCVector3* P = m_points[j].cloud->getPoint(m_points[j].index);
            //QString title = getName() + QString("#%0").arg(j);
            QString title = j == 0 ? getName() : "";
			context._win->display3DLabel(	title,
											*P + CCVector3(	context.labelMarkerTextShift,
															context.labelMarkerTextShift,
															context.labelMarkerTextShift),
											ccColor::white.rgba/*,
											font*/ ); //DGM: I get an OpenGL error if the font is defined this way?!
		}
        glPopAttrib();
	}

	if (pushName)
		glPopName();
}

//copied from cc2DLabel.cpp
//display parameters
static const int c_margin = 6;
static const int c_tabMarginX = 5;
static const int c_tabMarginY = 2;
static const int c_arrowBaseSize = 3;

//copied from cc2DLabel.cpp
//! Data table
struct Tab
{
	//! Default constructor
	Tab(int _maxBlockPerRow = 2)
		: maxBlockPerRow(_maxBlockPerRow)
		, blockCount(0)
		, rowCount(0)
		, colCount(0)
	{}

	//! Sets the maximum number of blocks per row
	/** \warning Must be called before adding data!
	**/
	inline void setMaxBlockPerRow(int maxBlock) { maxBlockPerRow = maxBlock; }

	//! Adds a 2x3 block (must be filled!)
	int add2x3Block()
	{
		//add columns (if necessary)
		if (colCount < maxBlockPerRow*2)
		{
			colCount += 2;
			colContent.resize(colCount);
			colWidth.resize(colCount,0);
		}
		int blockCol = (blockCount % maxBlockPerRow);
		//add new row
		if (blockCol == 0)
			rowCount += 3;
		++blockCount;

		//return the first column index of the block
		return blockCol*2;
	}

	//! Updates columns width table
	/** \return the total width
	**/
	int updateColumnsWidthTable(const QFontMetrics& fm)
	{
		//compute min width of each column
		int totalWidth = 0;
		for (int i=0; i<colCount; ++i)
		{
			int maxWidth = 0;
			for (int j=0; j<colContent[i].size(); ++j)
				maxWidth = std::max(maxWidth, fm.width(colContent[i][j]));
			colWidth[i] = maxWidth;
			totalWidth += maxWidth;
		}
		return totalWidth;
	}

	//! Maximum number of blocks per row
	int maxBlockPerRow;
	//! Number of 2x3 blocks
	int blockCount;
	//! Number of rows
	int rowCount;
	//! Number of columns
	int colCount;
	//! Columns width
	std::vector<int> colWidth;
	//! Columns content
	std::vector<QStringList> colContent;
};

void MarkedLine::drawMeOnly2D(CC_DRAW_CONTEXT& context)
{
    //测试
    //return;

	if (!m_dispIn2D)
		return;

	assert(!m_points.empty());

	//standard case: list names pushing
	bool pushName = MACRO_DrawEntityNames(context);
	if (pushName)
		glPushName(getUniqueID());

	//we should already be in orthoprojective & centered omde
	//glOrtho(-halfW,halfW,-halfH,halfH,-maxS,maxS);

	//label title
	const int precision = context.dispNumberPrecision;
	//QString title = getMarkedType()->getName() + "/" + getName(); //getTitle(precision);
    QString title = getName(); //getTitle(precision);

//#define DRAW_CONTENT_AS_TAB
#ifdef DRAW_CONTENT_AS_TAB
	//draw contents as an array
	Tab tab(4);
#else
//simply display the content as text
	QStringList body;
#endif

	int rowHeight = 0;
	int titleHeight = 0;
	GLdouble arrowDestX = -1.0, arrowDestY = -1.0;
	QFont bodyFont,titleFont;
	if (!pushName)
	{
		/*** line from 2D point to label ***/

		//compute arrow head position
		CCVector3 arrowDest;
		m_points[0].cloud->getPoint(m_points[0].index,arrowDest);

		//project it in 2D screen coordinates
		int VP[4];
		context._win->getViewportArray(VP);
		const double* MM = context._win->getModelViewMatd(); //viewMat
		const double* MP = context._win->getProjectionMatd(); //projMat
		GLdouble zp;
		gluProject(arrowDest.x,arrowDest.y,arrowDest.z,MM,MP,VP,&arrowDestX,&arrowDestY,&zp);

		/*** label border ***/
		bodyFont = context._win->getLabelDisplayFont(); //takes rendering zoom into account!
		titleFont = bodyFont; //takes rendering zoom into account!
		//titleFont.setBold(true);

		QFontMetrics titleFontMetrics(titleFont);
		titleHeight = titleFontMetrics.height();

		QFontMetrics bodyFontMetrics(bodyFont);
		rowHeight = bodyFontMetrics.height();

		//get label box dimension
		int dx = 100;
		int dy = 0;
		{
			//base box dimension
			dx = std::max(dx,titleFontMetrics.width(title));
			dy += c_margin;		//top vertical margin
			dy += titleHeight;	//title
            dy += c_margin;		//bottom vertical margin of title

			if (m_showFullBody)
			{
#ifdef DRAW_CONTENT_AS_TAB
				try
				{
                    //TODO 修改2D标签的显示内容
                    LabelInfo1 info;
                    getLabelInfo1(info);

                    bool isShifted = info.cloud->isShifted();
                    //1st block: X, Y, Z (local)
                    {
                        int c = tab.add2x3Block();
                        QChar suffix;
                        if (isShifted)
                            suffix = 'l'; //'l' for local
                        const CCVector3* P = info.cloud->getPoint(info.pointIndex);
                        tab.colContent[c] << QString("X")+suffix; tab.colContent[c+1] << QString::number(P->x,'f',precision);
                        tab.colContent[c] << QString("Y")+suffix; tab.colContent[c+1] << QString::number(P->y,'f',precision);
                        tab.colContent[c] << QString("Z")+suffix; tab.colContent[c+1] << QString::number(P->z,'f',precision);
                    }
                    //next block:  X, Y, Z (global)
                    if (isShifted)
                    {
                        int c = tab.add2x3Block();
                        CCVector3d P = info.cloud->toGlobal3d(*info.cloud->getPoint(info.pointIndex));
                        tab.colContent[c] << "Xg"; tab.colContent[c+1] << QString::number(P.x,'f',precision);
                        tab.colContent[c] << "Yg"; tab.colContent[c+1] << QString::number(P.y,'f',precision);
                        tab.colContent[c] << "Zg"; tab.colContent[c+1] << QString::number(P.z,'f',precision);
                    }
                    //next block: normal
                    if (info.hasNormal)
                    {
                        int c = tab.add2x3Block();
                        tab.colContent[c] << "Nx"; tab.colContent[c+1] << QString::number(info.normal.x,'f',precision);
                        tab.colContent[c] << "Ny"; tab.colContent[c+1] << QString::number(info.normal.y,'f',precision);
                        tab.colContent[c] << "Nz"; tab.colContent[c+1] << QString::number(info.normal.z,'f',precision);
                    }

                    //next block: RGB color
                    if (info.hasRGB)
                    {
                        int c = tab.add2x3Block();
                        tab.colContent[c] <<"R"; tab.colContent[c+1] << QString::number(info.rgb.x);
                        tab.colContent[c] <<"G"; tab.colContent[c+1] << QString::number(info.rgb.y);
                        tab.colContent[c] <<"B"; tab.colContent[c+1] << QString::number(info.rgb.z);
                    }
				}
				catch(std::bad_alloc)
				{
					//not enough memory
					return;
				}

				//compute min width of each column
				int totalWidth = tab.updateColumnsWidthTable(bodyFontMetrics);

				int tabWidth = totalWidth + tab.colCount * (2*c_tabMarginX); //add inner margins
				dx = std::max(dx,tabWidth);
				dy += tab.rowCount * (rowHeight + 2*c_tabMarginY); //add inner margins
				//we also add a margin every 3 rows
				dy += std::max(0,(tab.rowCount/3)-1) * c_margin;
				dy += c_margin;		//bottom vertical margin
#else
				body = getLabelContent(precision);
				if (!body.empty())
				{
					dy += c_margin;	//vertical margin above separator
					for (int j=0; j<body.size(); ++j)
					{
						dx = std::max(dx,bodyFontMetrics.width(body[j]));
						dy += rowHeight; //body line height
					}
					dy += c_margin;	//vertical margin below text
				}
#endif //DRAW_CONTENT_AS_TAB
			}

			dx += c_margin*2;	// horizontal margins
		}

		//main rectangle
		m_labelROI = QRect(0,0,dx,dy);

		//close button
		//m_closeButtonROI.right()   = dx-c_margin;
		//m_closeButtonROI.left()    = m_closeButtonROI.right()-c_buttonSize;
		//m_closeButtonROI.bottom()  = c_margin;
		//m_closeButtonROI.top()     = m_closeButtonROI.bottom()+c_buttonSize;

		//automatically elide the title
		//title = titleFontMetrics.elidedText(title,Qt::ElideRight,m_closeButtonROI[0]-2*c_margin);
	}

	int halfW = (context.glW >> 1);
	int halfH = (context.glH >> 1);

	//draw label rectangle
	int xStart = static_cast<int>(static_cast<float>(context.glW) * m_screenPos[0]);
	int yStart = static_cast<int>(static_cast<float>(context.glH) * (1.0f-m_screenPos[1]));

	m_lastScreenPos[0] = xStart;
	m_lastScreenPos[1] = yStart - m_labelROI.height();

	//colors
	bool highlighted = (!pushName && isSelected());
	//default background color
	unsigned char alpha = static_cast<unsigned char>((context.labelOpacity/100.0) * 255);
	ccColor::Rgbaub defaultBkgColor(ccColor::Rgbaub((mColor.red()+255*2)/3, (mColor.green()+255*2)/3, (mColor.blue()+255*2)/3, 200)); //context.labelDefaultBkgCol,alpha);
	//default border color (mustn't be totally transparent!)
	ccColor::Rgbaub defaultBorderColor(ccColor::Rgbaub(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha()));
	if (!highlighted)
	{
		//apply only half of the transparency
		unsigned char halfAlpha = static_cast<unsigned char>((50.0 + context.labelOpacity/200.0) * 255);
		defaultBorderColor = ccColor::Rgbaub(context.labelDefaultBkgCol,halfAlpha);
	}

	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(static_cast<GLfloat>(-halfW+xStart),static_cast<GLfloat>(-halfH+yStart),0);

	if (!pushName)
	{
		//compute arrow base position relatively to the label rectangle (for 0 to 8)
		int arrowBaseConfig = 0;
		int iArrowDestX = static_cast<int>(arrowDestX)-xStart;
		int iArrowDestY = static_cast<int>(arrowDestY)-yStart;
		{
			if (iArrowDestX < m_labelROI.left()) //left
				arrowBaseConfig += 0;
			else if (iArrowDestX > m_labelROI.right()) //Right
				arrowBaseConfig += 2;
			else  //Middle
				arrowBaseConfig += 1;

			if (iArrowDestY > -m_labelROI.top()) //Top
				arrowBaseConfig += 0;
			else if (iArrowDestY < -m_labelROI.bottom()) //Bottom
				arrowBaseConfig += 6;
			else  //Middle
				arrowBaseConfig += 3;
		}

		//we make the arrow base start from the nearest corner
		if (arrowBaseConfig != 4) //4 = label above point!
		{
			glColor4ubv(defaultBorderColor.rgba);
			glBegin(GL_TRIANGLE_FAN);
			glVertex2d(arrowDestX-xStart,arrowDestY-yStart);
			switch(arrowBaseConfig)
			{
			case 0: //top-left corner
				glVertex2i(m_labelROI.left(), -m_labelROI.top()-2*c_arrowBaseSize);
				glVertex2i(m_labelROI.left(), -m_labelROI.top());
				glVertex2i(m_labelROI.left()+2*c_arrowBaseSize, -m_labelROI.top());
				break;
			case 1: //top-middle edge
				glVertex2i(std::max(m_labelROI.left(),iArrowDestX-c_arrowBaseSize), -m_labelROI.top());
				glVertex2i(std::min(m_labelROI.right(),iArrowDestX+c_arrowBaseSize), -m_labelROI.top());
				break;
			case 2: //top-right corner
				glVertex2i(m_labelROI.right(), -m_labelROI.top()-2*c_arrowBaseSize);
				glVertex2i(m_labelROI.right(), -m_labelROI.top());
				glVertex2i(m_labelROI.right()-2*c_arrowBaseSize, -m_labelROI.top());
				break;
			case 3: //middle-left edge
				glVertex2i(m_labelROI.left(), std::min(-m_labelROI.top(),iArrowDestY+c_arrowBaseSize));
				glVertex2i(m_labelROI.left(), std::max(-m_labelROI.bottom(),iArrowDestY-c_arrowBaseSize));
				break;
			case 4: //middle of rectangle!
				break;
			case 5: //middle-right edge
				glVertex2i(m_labelROI.right(), std::min(-m_labelROI.top(),iArrowDestY+c_arrowBaseSize));
				glVertex2i(m_labelROI.right(), std::max(-m_labelROI.bottom(),iArrowDestY-c_arrowBaseSize));
				break;
			case 6: //bottom-left corner
				glVertex2i(m_labelROI.left(), -m_labelROI.bottom()+2*c_arrowBaseSize);
				glVertex2i(m_labelROI.left(), -m_labelROI.bottom());
				glVertex2i(m_labelROI.left()+2*c_arrowBaseSize, -m_labelROI.bottom());
				break;
			case 7: //bottom-middle edge
				glVertex2i(std::max(m_labelROI.left(),iArrowDestX-c_arrowBaseSize), -m_labelROI.bottom());
				glVertex2i(std::min(m_labelROI.right(),iArrowDestX+c_arrowBaseSize), -m_labelROI.bottom());
				break;
			case 8: //bottom-right corner
				glVertex2i(m_labelROI.right(), -m_labelROI.bottom()+2*c_arrowBaseSize);
				glVertex2i(m_labelROI.right(), -m_labelROI.bottom());
				glVertex2i(m_labelROI.right()-2*c_arrowBaseSize, -m_labelROI.bottom());
				break;
			}
			glEnd();
		}
	}

	//main rectangle
	glColor4ubv(defaultBkgColor.rgba);
	glBegin(GL_QUADS);
	glVertex2i(m_labelROI.left(),  -m_labelROI.top());
	glVertex2i(m_labelROI.left(),  -m_labelROI.bottom());
	glVertex2i(m_labelROI.right(), -m_labelROI.bottom());
	glVertex2i(m_labelROI.right(), -m_labelROI.top());
	glEnd();

	//if (highlighted)
	{
		glPushAttrib(GL_LINE_BIT);
		glLineWidth(3.0f);
		glColor4ubv(defaultBorderColor.rgba);
		glBegin(GL_LINE_LOOP);
		glVertex2i(m_labelROI.left(),  -m_labelROI.top());
		glVertex2i(m_labelROI.left(),  -m_labelROI.bottom());
		glVertex2i(m_labelROI.right(), -m_labelROI.bottom());
		glVertex2i(m_labelROI.right(), -m_labelROI.top());
		glEnd();
		glPopAttrib();
	}

	//draw close button
	/*glColor3ubv(ccColor::black);
	glBegin(GL_LINE_LOOP);
	glVertex2i(m_closeButtonROI.left(),-m_closeButtonROI.top());
	glVertex2i(m_closeButtonROI.left(),-m_closeButtonROI.bottom());
	glVertex2i(m_closeButtonROI.right(),-m_closeButtonROI.bottom());
	glVertex2i(m_closeButtonROI.right(),-m_closeButtonROI.top());
	glEnd();
	glBegin(GL_LINES);
	glVertex2i(m_closeButtonROI.left()+2,-m_closeButtonROI.top()+2);
	glVertex2i(m_closeButtonROI.right()-2,-m_closeButtonROI.bottom()-2);
	glVertex2i(m_closeButtonROI.right()-2,-m_closeButtonROI.top()+2);
	glVertex2i(m_closeButtonROI.left()+2,-m_closeButtonROI.bottom()-2);
	glEnd();
	//*/

	//display text
	if (!pushName)
	{
		int xStartRel = c_margin;
		int yStartRel = 0;
        yStartRel -= c_margin;
        yStartRel -= titleHeight;

		ccColor::Rgbub defaultTextColor;
		if (context.labelOpacity < 40)
		{
			//under a given opacity level, we use the default text color instead!
			defaultTextColor = context.textDefaultCol;
		}
		else
		{
			defaultTextColor = ccColor::Rgbub(	255 - context.labelDefaultBkgCol.r,
												255 - context.labelDefaultBkgCol.g,
												255 - context.labelDefaultBkgCol.b);
		}

		//label title
		context._win->displayText(title,xStart+xStartRel,yStart+yStartRel,ccGenericGLDisplay::ALIGN_DEFAULT,0,defaultTextColor.rgb,&titleFont);
		yStartRel -= c_margin;
		
		if (m_showFullBody)
		{
#ifdef DRAW_CONTENT_AS_TAB
			int xCol = xStartRel;
			for (int c=0; c<tab.colCount; ++c)
			{
				int width = tab.colWidth[c] + 2*c_tabMarginX;
				int height = rowHeight + 2*c_tabMarginY;

				int yRow = yStartRel;
				int actualRowCount = std::min(tab.rowCount,tab.colContent[c].size());
				for (int r=0; r<actualRowCount; ++r)
				{
					if (r && (r % 3) == 0)
						yRow -= c_margin;

					//background color
					const unsigned char* backgroundColor = 0;
					const unsigned char* textColor = defaultTextColor.rgb;
					bool numericCol = ((c & 1) == 1);
					if (numericCol)
					{
						//textColor = defaultTextColor.rgb;
						//backgroundColor = ccColor::white; //no need to draw a background!
					}
					else
					{
						textColor = ccColor::white.rgba;
						int rgbIndex = (r % 3);
						if (rgbIndex == 0)
							backgroundColor = ccColor::red.rgba;
						else if (rgbIndex == 1)
							backgroundColor = ccColor::green.rgba; //darkGreen;
						else if (rgbIndex == 2)
							backgroundColor = ccColor::blue.rgba;
					}

					//draw background
					if (backgroundColor)
					{
						glColor3ubv(backgroundColor);
						glBegin(GL_QUADS);
						glVertex2i(m_labelROI.left() + xCol, -m_labelROI.top() + yRow);
						glVertex2i(m_labelROI.left() + xCol, -m_labelROI.top() + yRow - height);
						glVertex2i(m_labelROI.left() + xCol + width, -m_labelROI.top() + yRow - height);
						glVertex2i(m_labelROI.left() + xCol + width, -m_labelROI.top() + yRow);
						glEnd();
					}

					const QString& str = tab.colContent[c][r];

					int xShift = 0;
					if (numericCol)
					{
						//align digits on the right
						xShift = tab.colWidth[c] - QFontMetrics(bodyFont).width(str);
					}
					else
					{
						//align characetrs in the middle
						xShift = (tab.colWidth[c] - QFontMetrics(bodyFont).width(str)) / 2;
					}

					context._win->displayText(	str,
												xStart+xCol+c_tabMarginX+xShift,
												yStart+yRow-rowHeight,ccGenericGLDisplay::ALIGN_DEFAULT,0,textColor,&bodyFont);

					yRow -= height;
				}

				xCol += width;
			}
#else
			if (!body.empty())
			{
				//display body
				yStartRel -= c_margin;
				for (int i=0; i<body.size(); ++i)
				{
					yStartRel -= rowHeight;
					context._win->displayText(body[i],xStart+xStartRel,yStart+yStartRel,ccGenericGLDisplay::ALIGN_DEFAULT,0,defaultTextColor.rgb,&bodyFont);
				}
			}
#endif //DRAW_CONTENT_AS_TAB
		}
	}

	glPopAttrib();

	glPopMatrix();

	if (pushName)
		glPopName();
}

void MarkedLine::updateName()
{
    //ccObject::setName(mMarkedName + QString("(%1 control points)").arg(cc2DLabel::size()));
}

QStringList MarkedLine::getLabelContent(int precision)
{
    //重定义要显示在2D标签上的内容
    QStringList body;

    body << QString("Vertex number: %1").arg(cc2DLabel::size());

    body << QString("Length: %1").arg(getLineLength(), 0, 'f', precision);

    QVector3D startPoint = getStartPoint();
    body << QString("Start point: (%1, %2, %3)").arg(startPoint.x(), 0, 'f', precision).arg(startPoint.y(), 0, 'f', precision).arg(startPoint.z(), 0, 'f', precision);

    QVector3D endPoint = getEndPoint();
    body << QString("End   point: (%1, %2, %3)").arg(endPoint.x(), 0, 'f', precision).arg(endPoint.y(), 0, 'f', precision).arg(endPoint.z(), 0, 'f', precision);

    return body;
}

QVector3D MarkedLine::getStartPoint()
{
    const PickedPoint pickedPoint = cc2DLabel::getPoint(0);
    CCVector3d point = pickedPoint.cloud->toGlobal3d(*pickedPoint.cloud->getPoint(pickedPoint.index));
    return QVector3D(point.x, point.y, point.z);
}

QVector3D MarkedLine::getEndPoint()
{
    const PickedPoint pickedPoint = cc2DLabel::getPoint(cc2DLabel::size() - 1);
    CCVector3d point = pickedPoint.cloud->toGlobal3d(*pickedPoint.cloud->getPoint(pickedPoint.index));
    return QVector3D(point.x, point.y, point.z);
}

float MarkedLine::getLineLength()
{
    return mLineLength;
}

void MarkedLine::setShortestPathComputer(ShortestPathComputer *shortestPathComputer)
{
    mShortestPathComputer = shortestPathComputer;
}

void MarkedLine::undo()
{
    if (mCurrentPositionInHistory > 0)
    {
        mCurrentPositionInHistory--;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mLineLength = mHistory.at(mCurrentPositionInHistory).lineLength;
    }
    else if (mCurrentPositionInHistory == 0)
    {
        mCurrentPositionInHistory--;
        m_points.clear();
        mLineLength = 0.0;
    }
    else
    {
        return;
    }
}

void MarkedLine::redo()
{
    if (mCurrentPositionInHistory < mHistory.size() - 1)
    {
        mCurrentPositionInHistory++;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mLineLength = mHistory.at(mCurrentPositionInHistory).lineLength;
    }
    else
    {
        return;
    }
}
