//------------------------------------------------------------------------------
// emJpegImageFileModel.cpp
//
// Copyright (C) 2004-2009 Oliver Hamann.
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

#include <emJpeg/emJpegImageFileModel.h>
#include <setjmp.h>
extern "C" {
#	include <jerror.h>
#	include <jpeglib.h>
}


extern "C" {
	struct emJpegLoadingState {
		int imagePrepared;
		jpeg_decompress_struct cinfo;
		int cinfo_initialized;
		struct jpeg_error_mgr err;
		jmp_buf jmpbuffer;
		char errorText[JMSG_LENGTH_MAX+1];
		FILE * file;
		int y;
	};

	METHODDEF(void) emJpeg_error_exit(j_common_ptr cinfo)
	{
		struct emJpegLoadingState * L;

		L=(struct emJpegLoadingState *)cinfo->client_data;
		L->errorText[0]=0;
		(*cinfo->err->output_message)(cinfo);
		if (!L->errorText[0]) {
			strcpy(L->errorText,"Failed to read JPEG file.");
		}
		longjmp(L->jmpbuffer,1);
	}

	METHODDEF(void) emJpeg_output_message(j_common_ptr cinfo)
	{
		struct emJpegLoadingState * L;

		L=(struct emJpegLoadingState *)cinfo->client_data;
		(*cinfo->err->format_message)(cinfo, L->errorText);
	}
}


emRef<emJpegImageFileModel> emJpegImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emJpegImageFileModel,context,name,common)
}


emJpegImageFileModel::emJpegImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emJpegImageFileModel::~emJpegImageFileModel()
{
	emJpegImageFileModel::QuitLoading();
	emJpegImageFileModel::QuitSaving();
}


void emJpegImageFileModel::TryStartLoading() throw(emString)
{
	jpeg_saved_marker_ptr smp;
	const char * csstr;

	L=new emJpegLoadingState;
	memset(L,0,sizeof(emJpegLoadingState));

	L->file=fopen(GetFilePath(),"rb");
	if (!L->file) throw emGetErrorText(errno);

	if (setjmp(L->jmpbuffer)) throw emString(L->errorText);

	L->cinfo.client_data=L;
	L->cinfo.err=jpeg_std_error(&L->err);
	L->err.error_exit = emJpeg_error_exit;
	L->err.output_message = emJpeg_output_message;

	jpeg_create_decompress(&L->cinfo);
	L->cinfo_initialized=1;
	jpeg_stdio_src(&L->cinfo,L->file);

	jpeg_save_markers(&L->cinfo,JPEG_COM,0xffff);

	jpeg_read_header(&L->cinfo,TRUE);

	for (smp=L->cinfo.marker_list; smp; smp=smp->next) {
		if (smp->marker==JPEG_COM) {
			Comment=emString(
				(const char*)smp->data,
				smp->data_length
			);
		}
	}

	switch (L->cinfo.jpeg_color_space) {
	case JCS_GRAYSCALE:
		csstr="monochrome";
		L->cinfo.out_color_space=JCS_GRAYSCALE;
		break;
	case JCS_RGB:
		csstr="RGB";
		L->cinfo.out_color_space=JCS_RGB;
		break;
	case JCS_YCbCr:
		csstr="YUV";
		L->cinfo.out_color_space=JCS_RGB;
		break;
	case JCS_CMYK:
		csstr="CMYK";
		L->cinfo.out_color_space=JCS_RGB;
		break;
	case JCS_YCCK:
		csstr="YCCK";
		L->cinfo.out_color_space=JCS_RGB;
		break;
	default:
		csstr="unknown";
		L->cinfo.out_color_space=JCS_RGB;
		break;
	}

	FileFormatInfo=emString::Format("JPEG (%s)",csstr);

	Signal(ChangeSignal);

	L->cinfo.scale_num=1;
	L->cinfo.scale_denom=1;
	L->cinfo.output_gamma=1.0;
	L->cinfo.raw_data_out=FALSE;
	L->cinfo.quantize_colors=FALSE;
	L->cinfo.dct_method=JDCT_FLOAT;
	jpeg_start_decompress(&L->cinfo);

	if (L->cinfo.output_components!=1 &&
	    L->cinfo.output_components!=3) {
		throw emString("Unsupported JPEG file format.");
	}
}


bool emJpegImageFileModel::TryContinueLoading() throw(emString)
{
	JSAMPROW row;

	if (!L->imagePrepared) {
		Image.Setup(
			L->cinfo.output_width,
			L->cinfo.output_height,
			L->cinfo.output_components
		);
		L->imagePrepared=1;
		Signal(ChangeSignal);
	}

	if (setjmp(L->jmpbuffer)) throw emString(L->errorText);

	if (L->y<Image.GetHeight()) {
		row=
			(JSAMPROW)Image.GetWritableMap()+
			L->y*Image.GetWidth()*Image.GetChannelCount()
		;
		jpeg_read_scanlines(&L->cinfo,&row,1);
		L->y++;
		Signal(ChangeSignal);
	}

	if (L->y>=Image.GetHeight()) {
		jpeg_finish_decompress(&L->cinfo);
		return true;
	}

	return false;
}


void emJpegImageFileModel::QuitLoading()
{
	if (L) {
		if (L->cinfo_initialized) {
			if (!setjmp(L->jmpbuffer)) {
				jpeg_destroy_decompress(&L->cinfo);
			}
		}
		if (L->file) fclose(L->file);
		delete L;
		L=NULL;
	}
}


void emJpegImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emJpegImageFileModel: Saving not implemented.");
}


bool emJpegImageFileModel::TryContinueSaving() throw(emString)
{
	return false;
}


void emJpegImageFileModel::QuitSaving()
{
}


emUInt64 emJpegImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return
			((emUInt64)L->cinfo.output_width)*
			L->cinfo.output_height*
			L->cinfo.output_components+
			Comment.GetLen()
		;
	}
	else {
		return
			((emUInt64)Image.GetWidth())*
			Image.GetHeight()*
			Image.GetChannelCount()+
			Comment.GetLen()
		;
	}
}


double emJpegImageFileModel::CalcFileProgress()
{
	if (L && L->cinfo.output_height>0) {
		return 100.0*L->y/L->cinfo.output_height;
	}
	else {
		return 0.0;
	}
}
