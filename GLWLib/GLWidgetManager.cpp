#include "GLWidgetManager.h"
#include <QOpenGLWidget>
#include <assert.h>

CGLWidgetManager* CGLWidgetManager::m_pmgr = 0;

CGLWidgetManager* CGLWidgetManager::GetInstance()
{
	if (m_pmgr == 0) m_pmgr = new CGLWidgetManager;
	return m_pmgr;
}

void CGLWidgetManager::AttachToView(QOpenGLWidget *pview)
{
	assert(pview);
	m_pview = pview;
}

CGLWidgetManager::CGLWidgetManager()
{
	m_layer = 0;
}

CGLWidgetManager::CGLWidgetManager(const CGLWidgetManager& m) {}

CGLWidgetManager::~CGLWidgetManager()
{

}

void CGLWidgetManager::SetActiveLayer(int l)
{
	m_layer = l;
}
// Make sure widget are within bounds. (Call when parent QOpenGLWidget changes size)
void CGLWidgetManager::CheckWidgetBounds()
{
	// make sure we have a view
	if (m_pview == nullptr) return;

	// get the view's dimensions
	int w = m_pview->width();
	int h = m_pview->height();

	// resize widgets
	for (int i = 0; i<Widgets(); ++i)
	{
		GLWidget* pw = m_Widget[i];

		// snap the widget if any of its align flags are set
		if (pw->GetSnap()) SnapWidget(pw);

		int x0 = pw->x();
		if (x0 < 0) x0 = 0;

		int y0 = pw->y();
		if (y0 < 0) y0 = 0;

		int x1 = x0 + pw->w();
		if (x1 >= w) { x1 = w - 1; x0 = x1 - pw->w(); }
		if (x0 < 0) x0 = 0;

		int y1 = y0 + pw->h();
		if (y1 >= h) { y1 = h - 1; y0 = y1 - pw->h(); }
		if (y0 < 0) y0 = 0;

		pw->resize(x0, y0, x1 - x0, y1 - y0);
	}
}

void CGLWidgetManager::AddWidget(GLWidget* pw)
{
	pw->set_layer(m_layer);
	m_Widget.push_back(pw);
}

void CGLWidgetManager::RemoveWidget(GLWidget* pw)
{
	std::vector<GLWidget*>::iterator it = m_Widget.begin();
	for (int i=0; i<(int) m_Widget.size(); ++i, ++it)
	{
		if (m_Widget[i] == pw) 
		{
			m_Widget.erase(it);
			break;
		}
	}
}

int CGLWidgetManager::handle(int x, int y, int nevent)
{
	static int xp, yp;
	static int hp, fsp;
	static bool bresize = false;

	// see if there is a widget that wishes to handle this event
	// first we see if the user is trying to select a widget
	if (nevent == PUSH)
	{
		bool bsel = false;
		for (int i=0; i<(int) m_Widget.size(); ++i)
		{
			if (m_Widget[i]->visible() && m_Widget[i]->is_inside(x,y))
			{
				m_Widget[i]->set_focus();
				bsel = true;

/*				if (Fl::event_clicks()) 
				{
					m_Widget[i]->EditProperties();
					m_pview->Redraw();
				}
*/				break;
			}
		}
		if (!bsel) GLWidget::set_focus(0);
	}

	GLWidget* pw = GLWidget::get_focus();
	if (pw) 
	{
		switch (nevent)
		{
		case PUSH:
			{
				xp = x;
				yp = y;
				double r = (pw->m_x+pw->m_w-x)/10.0;
				double s = (pw->m_y+pw->m_h-y)/10.0;
				if ((r >= 0) && (s >= 0) && (r+s <= 1.0))
				{
					bresize = true; 
					hp = pw->m_h;
					fsp = pw->m_font.pointSize();
				}
				else bresize = false;

//				GLWidget::GetView()->Redraw();
			}
			return 1;
		case DRAG:
			if (pw->has_focus())
			{
				int x0 = pw->x();
				int y0 = pw->y();
				int w0 = pw->w();
				int h0 = pw->h();
				pw->align(0);

				if (bresize)
				{
					if (x0+w0+(x-xp) >= m_pview->width()) { pw->align(GLW_ALIGN_RIGHT); x = xp; }

					unsigned int n = pw->GetSnap();
					if (y0+h0+(y-yp) >= m_pview->height()) { pw->align(n | GLW_ALIGN_BOTTOM); y = yp; }

					pw->resize(x0, y0, w0 + (x-xp), h0 + (y - yp));

					int hn = pw->h();

					float ar = (float) hn / (float) hp;

					pw->m_font.setPointSize((int) (ar*fsp));
				}
				else
				{
					pw->resize(x0 + (x-xp), y0 + (y-yp), w0, h0);

					if (pw->x() <= 0) { pw->align(GLW_ALIGN_LEFT); x = xp; }
					else if (pw->x()+pw->w() >= m_pview->width()) { pw->align(GLW_ALIGN_RIGHT); x = xp; }

					unsigned int n = pw->GetSnap();
					if (pw->y() <= 0) { pw->align(n | GLW_ALIGN_TOP); y = yp; }
					else if (pw->y() + pw->h() >= m_pview->height()) { pw->align(n | GLW_ALIGN_BOTTOM); y = yp; }
				}

				SnapWidget(pw);

				xp = x;
				yp = y;

//				GLWidget::GetView()->Redraw();

/*				CWnd* pwnd = flxGetMainWnd();
				x0 = pw->x();
				y0 = pw->y();
				w0 = pw->w();
				h0 = pw->h();
				pwnd->SetStatusBar("%d,%d,%d,%d", x0, y0, w0, h0);
*/
			}
			return 1;
		case RELEASE:
			{
//				CWnd* pwnd = flxGetMainWnd();
//				pwnd->SetStatusBar(0);
				bresize = false;
			}
			break;
		}
	}

	return 0;
}

void CGLWidgetManager::SnapWidget(GLWidget* pw)
{
	assert(m_pview);

	int W = m_pview->width();
	int H = m_pview->height();

	int w = pw->w();
	int h = pw->h();

	unsigned int nflag = pw->GetSnap();
	if      (nflag & GLW_ALIGN_LEFT   ) pw->m_x = 0;
	else if (nflag & GLW_ALIGN_RIGHT  ) pw->m_x = W-w-1;
	else if (nflag & GLW_ALIGN_HCENTER) pw->m_x = W/2 - w/2;

	if      (nflag & GLW_ALIGN_TOP    ) pw->m_y = 0;
	else if (nflag & GLW_ALIGN_BOTTOM ) pw->m_y = H-h-1;
	else if (nflag & GLW_ALIGN_VCENTER) pw->m_y = H/2 - h/2;
}

void CGLWidgetManager::DrawWidgets(QPainter* painter)
{
	for (int i=0; i<(int) m_Widget.size(); ++i) 
	{
		GLWidget* pw = m_Widget[i];
		if (pw->visible() && ((pw->layer() == 0) || (pw->layer() == m_layer)))
		{
			// snap the widget if any of its align flags are set
			if (pw->GetSnap()) SnapWidget(pw);

			int x0 = pw->m_x;
			int y0 = pw->m_y;
			int x1 = pw->m_x + pw->m_w;
			int y1 = (pw->m_y+pw->m_h);


			if (pw->has_focus())
			{
				painter->fillRect(x0, y0, pw->m_w, pw->m_h, QBrush(QColor::fromRgb(200,200,200,128)));
				painter->setPen(QPen(QColor::fromRgb(0, 0, 128)));
				painter->drawLine(x1-10, y1-1, x1-1, y1-10);
				painter->drawLine(x1- 5, y1-1, x1-1, y1- 5);
				painter->drawRect(x0, y0, pw->m_w, pw->m_h);
			}
		}
	}

	for (int i=0; i<(int) m_Widget.size(); ++i) 
	{
		GLWidget* pw = m_Widget[i];
		if (pw->visible() && ((pw->layer() == 0)|| (pw->layer() == m_layer))) pw->draw(painter);
	}
}
