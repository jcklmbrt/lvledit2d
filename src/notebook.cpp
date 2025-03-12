
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

	EditorContext &edit = dp->editor;

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
	wxString &name = canvas->editor.GetName();

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

	EditorContext &edit = canvas->editor;
	edit.GetName() = file.GetName();

	bool ret = AddCanvas(canvas);

	if(ret) {
		ret &= edit.Load(file);
	}

	if(!ret) {
		int page = FindPage(canvas);
		DoRemovePage(page);
	}

	return ret;
}


void Notebook::OnPageClose(wxAuiNotebookEvent &e)
{
	e.Skip();

	HistoryList *hlist = HistoryList::GetInstance();
	TextureList *tlist = TextureList::GetInstance();

	if(GetPageCount() <= 0) {
		tlist->Unselect();
		hlist->SetItemCount(0);
		hlist->Refresh();
		tlist->SetItemCount(0);
		tlist->Refresh();
		MainFrame::GetInstance()->Sidebook_Hide();
	}
}


void Notebook::OnPageChange(wxAuiNotebookEvent &e)
{
	e.Skip();

	wxWindow *page = GetPage(e.GetSelection());
	GLCanvas *dp = dynamic_cast<GLCanvas *>(page);

	/* Jumping through hoops to keep our lists global
	   it would be much worse if we created a new object for every page */
	HistoryList *hlist = HistoryList::GetInstance();
	TextureList *tlist = TextureList::GetInstance();

	MainFrame::GetInstance()->Sidebook_Show();

	if(dp != nullptr) {
		size_t ntex = dp->editor.NumTextures();
		size_t nact = dp->editor.NumActions();
		tlist->SetItemCount(ntex);
		tlist->SetSelected(ntex - 1);
		hlist->SetItemCount(nact);
	}
	else {
		hlist->SetItemCount(0);
		tlist->SetItemCount(0);
		tlist->Unselect();
	}

	hlist->Refresh();
	tlist->Refresh();
}
