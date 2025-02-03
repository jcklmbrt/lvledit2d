
#include "src/edit/editorcontext.hpp"
#include "src/historylist.hpp"
#include "src/texturepanel.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glcontext.hpp"
#include "src/mainframe.hpp"
#include "src/notebook.hpp"


Notebook::Notebook(wxWindow *parent, HistoryList *hlist)
	: wxAuiNotebook(parent, wxID_ANY)
{
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &Notebook::OnPageChange, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Notebook::OnPageClose, this);
}


Notebook::~Notebook()
{
}


void Notebook::Save(bool force_rename)
{
	GLCanvas *dp = GLCanvas::GetCurrent();

	if(dp == nullptr) {
		return;
	}

	EditorContext &edit = dp->GetEditor();

	if(edit.File == nullptr || force_rename) {
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
	SetPageText(sel, edit.Name);
}


bool Notebook::AddCanvas(GLCanvas *canvas)
{
	wxString name = canvas->GetEditor().Name;

	GLContext *ctx;

	if(GLContext::IsNull()) {
		ctx = new GLContext(canvas);
	} else {
		ctx = GLContext::GetInstance();
	}

	canvas->SetCurrent(*ctx);

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

	HistoryList *hlist;

	if(GetPageCount() <= 0) {
		hlist = HistoryList::GetInstance();
		hlist->SetItemCount(0);
		hlist->Refresh();
		MainFrame::GetInstance()->Sidebook_Hide();
	}
}


void Notebook::OnPageChange(wxAuiNotebookEvent &e)
{
	e.Skip();

	wxWindow *page = GetPage(e.GetSelection());
	GLCanvas *dp = dynamic_cast<GLCanvas *>(page);

	HistoryList *hlist = HistoryList::GetInstance();
	TextureList *tlist = TextureList::GetInstance();

	MainFrame::GetInstance()->Sidebook_Show();

	if(dp != nullptr) {
		tlist->SetItemCount(dp->Editor.Textures.size());
		hlist->SetItemCount(dp->Editor.Actions.size());
	}
	else {
		hlist->SetItemCount(0);
		tlist->SetItemCount(0);
	}

	hlist->Refresh();
	tlist->Refresh();
}
