//------------------------------------------------------------------------------
// emPngImageFileModel.cpp
//
// Copyright (C) 2004-2009,2011 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <png.h>
#include <setjmp.h>
#include <emPng/emPngImageFileModel.h>


extern "C" {
	struct emPngLoadingState {
		int ImagePrepared;
		png_structp png_ptr;
		png_infop info_ptr;
		png_infop end_info_ptr;
		png_uint_32 width, height, bytes_per_pixel;
		int bit_depth, color_type, interlace_type;
		int number_of_passes;
		jmp_buf jmpbuffer;
		char errorText[256];
		FILE * file;
		int y,pass;
	};

	static void emPng_error_fn(png_structp png_ptr, png_const_charp error_msg)
	{
		struct emPngLoadingState * L;

		L=(struct emPngLoadingState *)png_get_error_ptr(png_ptr);
		if (!error_msg) error_msg="PNG error";
		snprintf(L->errorText,sizeof(L->errorText),"%s",error_msg);
		L->errorText[sizeof(L->errorText)-1]=0;
		longjmp(L->jmpbuffer,1);
	}

	static void emPng_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
	{
	}
}


emRef<emPngImageFileModel> emPngImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPngImageFileModel,context,name,common)
}


emPngImageFileModel::emPngImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emPngImageFileModel::~emPngImageFileModel()
{
	emPngImageFileModel::QuitLoading();
	emPngImageFileModel::QuitSaving();
}


void emPngImageFileModel::TryStartLoading() throw(emString)
{
	emString colTypeStr;
	int rowbytes,originalPixelSize;

	L=new emPngLoadingState;
	memset(L,0,sizeof(emPngLoadingState));

	L->file=fopen(GetFilePath(),"rb");
	if (!L->file) throw emGetErrorText(errno);

	if (setjmp(L->jmpbuffer)) throw emString(L->errorText);

	L->png_ptr=png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		(png_voidp)L,
		emPng_error_fn,
		emPng_warning_fn
	);
	if (!L->png_ptr) throw emString("PNG lib failed.");

	L->info_ptr=png_create_info_struct(L->png_ptr);
	if (!L->info_ptr) throw emString("PNG lib failed.");

	L->end_info_ptr=png_create_info_struct(L->png_ptr);
	if (!L->end_info_ptr) throw emString("PNG lib failed.");

	png_init_io(L->png_ptr, L->file);

	png_read_info(L->png_ptr, L->info_ptr);

	png_get_IHDR(
		L->png_ptr,
		L->info_ptr,
		&L->width,
		&L->height,
		&L->bit_depth,
		&L->color_type,
		&L->interlace_type,
		NULL,
		NULL
	);

	originalPixelSize=L->bit_depth;
	if ((L->color_type&PNG_COLOR_MASK_PALETTE)==0) {
		originalPixelSize*=png_get_channels(L->png_ptr,L->info_ptr);
	}

	if ((L->color_type&PNG_COLOR_MASK_COLOR)!=0) {
		colTypeStr="color";
	}
	else {
		colTypeStr="grayscale";
	}
	if ((L->color_type&PNG_COLOR_MASK_ALPHA)!=0) {
		colTypeStr+="-alpha";
	}
	if ((L->color_type&PNG_COLOR_MASK_PALETTE)!=0) {
		colTypeStr+="-palette";
	}

	png_set_expand(L->png_ptr);
	png_set_strip_16(L->png_ptr);
	png_set_packing(L->png_ptr);
	L->number_of_passes=png_set_interlace_handling(L->png_ptr);
	png_read_update_info(L->png_ptr, L->info_ptr);
	rowbytes=png_get_rowbytes(L->png_ptr,L->info_ptr);
	L->bytes_per_pixel=rowbytes/L->width;
	if (rowbytes%L->width!=0 || L->bytes_per_pixel<1 || L->bytes_per_pixel>4) {
		throw emString("Unsupported PNG format.");
	}

	FileFormatInfo=emString::Format(
		"PNG %d-bit %s (%d channels extracted)",
		originalPixelSize,
		colTypeStr.Get(),
		(int)L->bytes_per_pixel
	);
	Signal(ChangeSignal);
}


bool emPngImageFileModel::TryContinueLoading() throw(emString)
{
	png_textp t;
	int e,i,n;

	if (!L->ImagePrepared) {
		Image.Setup(
			L->width,
			L->height,
			L->bytes_per_pixel
		);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	if (setjmp(L->jmpbuffer)) throw emString(L->errorText);

	if (L->y<(int)L->height && L->pass<L->number_of_passes) {
		png_read_row(
			L->png_ptr,
			Image.GetWritableMap()+L->y*Image.GetWidth()*Image.GetChannelCount(),
			NULL
		);
		L->y++;
		if (L->y>=(int)L->height) {
			L->y=0;
			L->pass++;
		}
		Signal(ChangeSignal);
		return false;
	}

	png_read_end(L->png_ptr,L->end_info_ptr);

	for (e=0; e<2; e++) {
		n=png_get_text(
			L->png_ptr,
			e ? L->end_info_ptr : L->info_ptr,
			&t,
			NULL
		);
		for (i=0; i<n; i++) {
			if (
				t[i].text && *t[i].text && t[i].key && (
					strcasecmp(t[i].key,"Comment")==0 ||
					strcasecmp(t[i].key,"Description")==0
				)
			) {
				if (!Comment.IsEmpty()) Comment+='\n';
				Comment+=t[i].text;
			}
		}
	}
	Signal(ChangeSignal);

	return true;
}


void emPngImageFileModel::QuitLoading()
{
	if (L) {
		if (L->png_ptr) {
			png_destroy_read_struct(
				&L->png_ptr,
				L->info_ptr ? &L->info_ptr : NULL,
				L->end_info_ptr ? &L->end_info_ptr : NULL
			);
		}
		if (L->file) fclose(L->file);
		delete L;
		L=NULL;
	}
}


void emPngImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emPngImageFileModel: Saving not implemented.");
}


bool emPngImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emPngImageFileModel::QuitSaving()
{
}


emUInt64 emPngImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->width)*
		       L->height*
		       L->bytes_per_pixel;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emPngImageFileModel::CalcFileProgress()
{
	if (L && L->height>0) {
		return 100.0*L->y/L->height;
	}
	else {
		return 0.0;
	}
}
