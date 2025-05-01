#include "src/edit/editorcontext.hpp"
#include "src/historylist.hpp"
#include "src/texturepanel.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glcontext.hpp"
#include "src/mainframe.hpp"
#include "src/notebook.hpp"
#include "src/layerpanel.hpp"


Notebook::Notebook(wxWindow *parent, HistoryList *hlist)
    : wxAuiNotebook(parent, wxID_ANY) {
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &Notebook::OnPageChange, this);
	Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSED, &Notebook::OnPageClose, this);
}


Notebook::~Notebook()
{
}


void Notebook::Save(bool force_rename)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();

	if(canvas == nullptr) {
		return;
	}

	EditorContext &edit = canvas->GetEditor();

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
	wxString &name = canvas->GetEditor().GetName();

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
	GLCanvas *canvas = dynamic_cast<GLCanvas *>(page);

	/* Jumping through hoops to keep our lists global
	   it would be much worse if we created a new object for every page */
	HistoryList *hlist = HistoryList::GetInstance();
	TextureList *tlist = TextureList::GetInstance();
	LayerList *llist = LayerList::GetInstance();

	MainFrame::GetInstance()->Sidebook_Show();

	if(canvas != nullptr) {
		EditorContext &edit = canvas->GetEditor();
		size_t ntex = edit.GetTextures().size();
		size_t nact = edit.GetActList().TotalActions();
		size_t nlay = edit.GetLayers().size();
		edit.SetSelectedTextureIndex(ntex - 1);
		tlist->SetItemCount(ntex);
		hlist->SetItemCount(nact);
		llist->SetItemCount(nlay);
	}
	else {
		hlist->SetItemCount(0);
		tlist->SetItemCount(0);
		llist->SetItemCount(0);
	}

	hlist->Refresh();
	tlist->Refresh();
	llist->Refresh();
}
