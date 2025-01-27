
#include "src/edit/editorcontext.hpp"
#include "src/historylist.hpp"
#include "src/glcanvas.hpp"
#include "src/glcontext.hpp"
#include "src/notebook.hpp"


Notebook::Notebook(wxWindow *parent, HistoryList *hlist)
	: wxAuiNotebook(parent, wxID_ANY),
	  m_hlist(hlist)
{
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &Notebook::OnPageChange, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Notebook::OnPageClose, this);
}


Notebook::~Notebook()
{
	delete m_context;
}


void Notebook::Save(bool force_rename)
{
	GLCanvas *dp = GLCanvas::GetCurrent();

	if(dp == nullptr) {
		return;
	}

	EditorContext &edit = dp->GetEditor();

	if(!edit.HasFile() || force_rename) {
		wxFileDialog dialog(GetParent(), "Save file", "", "",
			"LvlEdit2d files (*.l2d)|*.l2d", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if(dialog.ShowModal() == wxID_CANCEL) {
			return;
		}

		wxString path = dialog.GetPath();
		if(!path.EndsWith(".l2d")) {
			path.append(".l2d");
		}
		edit.Save(path);
	}
	else {
		edit.Save();
	}

	int sel = GetSelection();
	SetPageText(sel, edit.GetName());
}


bool Notebook::AddCanvas(GLCanvas *canvas)
{
	wxString name = canvas->GetEditor().GetName();

	if(m_context == nullptr) {
		m_context = new GLContext(canvas);
	}

	canvas->SetContext(m_context);
	canvas->SetCurrent(*m_context);

	return AddPage(canvas, name, true);
}


bool Notebook::AddCanvas()
{
	wxGLAttributes attrs;
	attrs.PlatformDefaults().Defaults().EndList();
	GLCanvas *canvas = new GLCanvas(this, attrs);

	/* untitled */
	return AddCanvas(canvas);
}


bool Notebook::AddCanvas(const wxFileName &file)
{
	wxGLAttributes attrs;
	attrs.PlatformDefaults().Defaults().EndList();
	GLCanvas *canvas = new GLCanvas(this, attrs);

	EditorContext &edit = canvas->GetEditor();
	
	edit.Load(file);

	return AddCanvas(canvas);
}


void Notebook::OnPageClose(wxAuiNotebookEvent &e)
{
	e.Skip();

	if(GetPageCount() <= 0) {
		m_hlist->SetItemCount(0);
		m_hlist->Refresh();
	}
}


void Notebook::OnPageChange(wxAuiNotebookEvent &e)
{
	e.Skip();

	wxWindow *page = GetPage(e.GetSelection());
	GLCanvas *dp = dynamic_cast<GLCanvas *>(page);

	if(dp != nullptr) {
		m_hlist->SetItemCount(dp->GetEditor().GetHistory().size());
	}
	else {
		m_hlist->SetItemCount(0);
	}

	m_hlist->Refresh();
}
