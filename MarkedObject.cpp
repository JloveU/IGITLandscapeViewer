#include "MarkedObject.h"
#include "ccGenericGLDisplay.h"

MarkedObject::MarkedObject(QString name)
    : cc2DLabel(name)
    , mMarkedType(NULL)
    , mColor(255, 0, 0)
{
    setSelected(true);
    setVisible(true);
}

void MarkedObject::setColor(const QColor &color)
{
    mColor = color;
}

QColor MarkedObject::getColor()
{
    return mColor;
}

void MarkedObject::setMarkedType(ccHObject *markedType)
{
    mMarkedType = markedType;
}

ccHObject* MarkedObject::getMarkedType()
{
    return mMarkedType;
}

inline void MarkedObject::setSelected(bool state)
{
    //修改父类的选中与非选中的显示效果，是的选中与非选中的区别为是否显示2D标签（两种情况下均显示3D标志）
    setDisplayedIn2D(state);
    cc2DLabel::setSelected(state);
}

//copied from cc2DLabel.cpp
//display parameters
static const int c_margin = 6;
static const int c_tabMarginX = 5;
static const int c_tabMarginY = 2;
static const int c_arrowBaseSize = 3;
void MarkedObject::drawMeOnly2D(CC_DRAW_CONTEXT& context)
{
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

    QStringList body;

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
			}

			dx += c_margin*2;	// horizontal margins
		}

		//main rectangle
		m_labelROI = QRect(0,0,dx,dy);
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
		}
	}

	glPopAttrib();

	glPopMatrix();

	if (pushName)
		glPopName();
}
