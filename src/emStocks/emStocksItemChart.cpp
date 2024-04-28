//------------------------------------------------------------------------------
// emStocksItemChart.cpp
//
// Copyright (C) 2021-2022,2024 Oliver Hamann.
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

#include <emStocks/emStocksItemChart.h>
#include <emStocks/emStocksListBox.h>


emStocksItemChart::emStocksItemChart(
	ParentArg parent, const emString & name, emStocksListBox & listBox,
	emStocksConfig & config
)
	: emBorder(parent,name),
	ListBox(listBox),
	Config(config),
	DataUpToDate(false),
	StartYear(0),
	StartMonth(0),
	StartDay(0),
	TotalDays(1),
	DaysPerPrice(1),
	OwningShares(false),
	TradeOffsetDays(INT_MIN),
	XOffset(0.0),
	XFactor(1.0),
	YOffset(0.0),
	YFactor(-1.0),
	LowerPrice(0.0),
	UpperPrice(1.0)
{
	emLook look;

	TradePrice.Valid=false;
	PriceOnSelectedDate.Valid=false;
	DesiredPrice.Valid=false;
	Prices.SetTuningLevel(4);
	MinPrice.Valid=false;
	MaxPrice.Valid=false;

	UpdateTimeout=emGetClockMS()+emGetInt64Random(1000,3000);

	SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);

	look=GetLook();
	look.SetOutputBgColor(emColor::BLACK);
	SetLook(look);

	AddWakeUpSignal(Config.GetChangeSignal());
	AddWakeUpSignal(ListBox.GetSelectedDateSignal());
	WakeUp();
}


emStocksItemChart::~emStocksItemChart()
{
}


void emStocksItemChart::SetStockRec(
	emStocksRec::StockRec * stockRec
)
{
	if (GetStockRec()!=stockRec) {
		SetListenedRec(stockRec);
		InvalidateData();
	}
}


bool emStocksItemChart::Cycle()
{
	bool busy;

	busy=emBorder::Cycle();

	if (
		IsSignaled(Config.GetChangeSignal()) ||
		IsSignaled(ListBox.GetSelectedDateSignal())
	) {
		InvalidateData();
	}

	if (!DataUpToDate) {
		if (
			IsTimeSliceAtEnd() &&
			UpdateTimeout>emGetClockMS() &&
			(!IsViewed() || GetViewedWidth()<0.1*GetView().GetCurrentWidth())
		) return true;
		UpdateData();
	}

	return busy;
}


void emStocksItemChart::Notice(NoticeFlags flags)
{
	emBorder::Notice(flags);

	if (flags&NF_LAYOUT_CHANGED) {
		InvalidateData();
	}

	if (flags&NF_VIEWING_CHANGED) {
		if (DataUpToDate && DaysPerPrice!=CalculateDaysPerPrice()) {
			InvalidateData();
		}
	}
}


void emStocksItemChart::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	PaintXScaleLines(painter);
	PaintYScaleLines(painter);
	PaintXScaleLabels(painter);
	PaintYScaleLabels(painter);
	PaintPriceBar(painter);
	PaintDesiredPrice(painter);
	PaintGraph(painter);
}


void emStocksItemChart::OnRecChanged()
{
	InvalidateData();
}


void emStocksItemChart::InvalidateData()
{
	if (DataUpToDate) {
		DataUpToDate=false;
		UpdateTimeout=emGetClockMS()+emGetInt64Random(1000,3000);
		WakeUp();
	}
}


void emStocksItemChart::UpdateData()
{
	if (!DataUpToDate) {
		UpdateTimeRange();
		UpdatePrices1();
		UpdatePrices2();
		UpdateTransformation();
		DataUpToDate=true;
		InvalidatePainting();
	}
}


void emStocksItemChart::UpdateTimeRange()
{
	EndDate=ListBox.GetSelectedDate();
	if (!emStocksRec::ParseDate(EndDate)) EndDate=emStocksRec::GetCurrentDate();
	EndDate=emStocksFileModel::AddDaysToDate(1,EndDate);
	TotalDays=Config.CalculateChartPeriodDays(EndDate);
	StartDate=emStocksFileModel::AddDaysToDate(-TotalDays,EndDate);
	emStocksRec::ParseDate(StartDate,&StartYear,&StartMonth,&StartDay);
	DaysPerPrice=CalculateDaysPerPrice();
}


int emStocksItemChart::CalculateDaysPerPrice()
{
	int d,m;

	if (!IsViewed()) return TotalDays;
	m=emMin(
		TotalDays/2,
		(int)(TotalDays*ViewToPanelDeltaX(1.2))
	);
	for (d=1; d<m; d<<=1);
	return d;
}


void emStocksItemChart::UpdatePrices1()
{
	const emStocksRec::StockRec * stockRec;
	emString str;

	stockRec=GetStockRec();
	if (!stockRec || !IsViewed()) {
		OwningShares=false;
		TradePrice.Valid=false;
		TradePriceText.Clear();
		TradeOffsetDays=INT_MIN;
		PriceOnSelectedDate.Valid=false;
		PriceOnSelectedDateText.Clear();
		DesiredPrice.Valid=false;
		DesiredPriceText.Clear();
		MinPrice.Valid=false;
		MaxPrice.Valid=false;
		return;
	}

	OwningShares=stockRec->OwningShares.Get();

	TradePrice.Set(stockRec->TradePrice.Get());
	MinPrice=TradePrice;
	MaxPrice=TradePrice;
	if (TradePrice) {
		TradePriceText=emString::Format(
			"%s: %s",
			OwningShares ? "Purchase Price" : "Sale Price",
			stockRec->TradePrice.Get().Get()
		);
		if (!stockRec->TradeDate.Get().IsEmpty()) {
			TradeOffsetDays=emStocksFileModel::GetDateDifference(StartDate,stockRec->TradeDate.Get());
		}
		else {
			TradeOffsetDays=INT_MIN;
		}
	}
	else {
		TradePriceText.Clear();
		TradeOffsetDays=INT_MIN;
	}

	str=stockRec->GetPriceOfDate(ListBox.GetSelectedDate());
	PriceOnSelectedDate.Set(str);
	if (PriceOnSelectedDate) {
		if (!MinPrice || MinPrice.Value>PriceOnSelectedDate.Value) {
			MinPrice=PriceOnSelectedDate;
		}
		if (!MaxPrice || MaxPrice.Value<PriceOnSelectedDate.Value) {
			MaxPrice=PriceOnSelectedDate;
		}
		PriceOnSelectedDateText=emString::Format("Price: %s",str.Get());
	}
	else {
		PriceOnSelectedDateText.Clear();
	}

	DesiredPrice.Set(stockRec->DesiredPrice.Get());
	if (DesiredPrice) {
		if (!MinPrice || MinPrice.Value>DesiredPrice.Value) {
			MinPrice=DesiredPrice;
		}
		if (!MaxPrice || MaxPrice.Value<DesiredPrice.Value) {
			MaxPrice=DesiredPrice;
		}
		DesiredPriceText=emString::Format(
			"Desired Price: %s",
			stockRec->DesiredPrice.Get().Get()
		);
	}
	else {
		DesiredPriceText.Clear();
	}
}


void emStocksItemChart::UpdatePrices2()
{
	const emStocksRec::StockRec * stockRec;
	const char * s1, * s2;
	Price * t1, * t2;
	double sv,tv,minv,maxv;
	int diffDays,remainingDays,n;

	stockRec=GetStockRec();
	if (
		!stockRec || !IsViewed() ||
		stockRec->Prices.Get().IsEmpty() ||
		stockRec->LastPriceDate.Get().IsEmpty()
	) {
		Prices.Clear(true);
		return;
	}

	s1=stockRec->Prices.Get().Get();
	s2=s1+strlen(s1);

	Prices.SetCount((TotalDays+DaysPerPrice-1)/DaysPerPrice,true);
	t1=Prices.GetWritable();
	t2=t1+Prices.GetCount();
	memset(t1,0,(char*)t2-(char*)t1);

	remainingDays=(TotalDays-1)%DaysPerPrice+1;

	diffDays=emStocksFileModel::GetDateDifference(
		stockRec->LastPriceDate.Get(),
		EndDate
	);
	diffDays--;
	if (diffDays<0) {
		while (s1<s2) {
			s2--;
			if (*s2=='|') {
				diffDays++;
				if (diffDays>=0) break;
			}
		}
	}
	else if (diffDays>0) {
		t2-=diffDays/DaysPerPrice;
		remainingDays-=diffDays%DaysPerPrice;
		if (remainingDays<=0) {
			t2--;
			remainingDays+=DaysPerPrice;
		}
	}

	if (s1>=s2 || t1>=t2) return;

	minv=1E100;
	maxv=-1E100;
	tv=0.0;
	n=0;
	do {
		s2--;
		if (*s2!='|') {
			do { s2--; } while (s1<=s2 && *s2!='|');
			sv=atof(s2+1);
			tv+=sv;
			n++;
			if (minv>sv) minv=sv;
			if (maxv<sv) maxv=sv;
		}
		remainingDays--;
		if (remainingDays<=0) {
			t2--;
			if (n>0) {
				t2->Valid=true;
				t2->Value=tv/n;
			}
			if (t1>=t2) break;
			remainingDays=DaysPerPrice;
			tv=0.0;
			n=0;
		}
	} while (s1<s2);

	if (t1<t2 && n>0) {
		t2--;
		t2->Valid=true;
		t2->Value=tv/n;
	}

	if (minv<=maxv) {
		if (!MinPrice || MinPrice.Value>minv) {
			MinPrice.Valid=true;
			MinPrice.Value=minv;
		}
		if (!MaxPrice || MaxPrice.Value<maxv) {
			MaxPrice.Valid=true;
			MaxPrice.Value=maxv;
		}
	}
}


void emStocksItemChart::UpdateTransformation()
{
	double x,y,w,h,c,d,p1,p2;

	GetContentRect(&x,&y,&w,&h);
	d=h*0.008;
	y+=d;
	h-=2*d;

	XOffset=x;
	if (TotalDays>0) {
		XFactor=w/TotalDays;
	}
	else {
		XFactor=1.0;
	}

	if (MinPrice && MaxPrice) {
		c=0.0;
		if (TradePrice) c=TradePrice.Value;
		else if (DesiredPrice) c=DesiredPrice.Value;
		else c=(MinPrice.Value+MaxPrice.Value)*0.5;
		d=emMax(0.5*c,emMax(MaxPrice.Value-c,c-MinPrice.Value));
		p1=c-d;
		p2=c+d;
		if (p1<0.0) {
			p1=emMin(0.0,MinPrice.Value);
			p2=MaxPrice.Value;
		}
		p2=emMax(p2,p1+1E-6);
	}
	else {
		p1=0.0;
		p2=100.0001;
	}

	YFactor=h/(p1-p2);
	YOffset=y-YFactor*p2;
	LowerPrice=p1;
	UpperPrice=p2;
}


void emStocksItemChart::PaintXScaleLines(const emPainter & painter) const
{
	double maxThickness,fDay,fEndDay,x,y,h,t,f;
	int year,month,mday,day,endDay,minLevel,y10;
	emColor c;

	f=ViewToPanelDeltaX(14.0)/XFactor;
	if      (f<=  1.0 ) minLevel=0; // days
	else if (f<= 30.4 ) minLevel=1; // months
	else if (f<=365.25) minLevel=2; // years
	else if (f<=3652.5) minLevel=3; // 10 years
	else return;

	maxThickness=emMin(0.002,ViewToPanelDeltaX(2.6));

	fDay=emMax(
		0.0,
		(painter.GetUserClipX1()-XOffset-maxThickness*0.5)/XFactor
	);
	fEndDay=emMin(
		(double)TotalDays,
		(painter.GetUserClipX2()-XOffset+maxThickness*0.5)/XFactor
	);
	if (fDay>fEndDay) return;
	day=(int)ceil(fDay);
	endDay=(int)fEndDay;

	year=StartYear;
	month=StartMonth;
	mday=StartDay;
	emStocksFileModel::AddDaysToDate(day,&year,&month,&mday);

	if (minLevel>0) {
		if (mday>1) {
			day+=emStocksFileModel::GetDaysOfMonth(year,month)-mday+1;
			mday=1;
			month++;
			if (month>12) { year++; month=1; }
		}
		if (minLevel>1) {
			if (month>1) {
				day+=emStocksFileModel::GetDateDifference(year,month,1,year+1,1,1);
				year++;
				month=1;
			}
			if (minLevel>2) {
				if (year%10!=0) {
					y10=year+10-year%10;
					day+=emStocksFileModel::GetDateDifference(year,1,1,y10,1,1);
					year=y10;
				}
			}
		}
	}

	y=YOffset+YFactor*UpperPrice;
	h=YFactor*(LowerPrice-UpperPrice);
	c=emColor(128,128,128);

	while (day<=endDay) {
		x=XOffset+XFactor*day;

		t=0.01;
		if (mday==1) {
			t=0.01*30.4;
			if (month==1) {
				t=0.01*365.25;
				if (year%10==0) t=0.01*3652.5;
			}
		}
		t*=XFactor;
		if (t>maxThickness) t=maxThickness;
		painter.PaintRect(x-t*0.5,y,t,h,c);

		if (minLevel==0) {
			day++;
			mday++;
			if (mday>emStocksFileModel::GetDaysOfMonth(year,month)) {
				mday=1;
				month++;
				if (month>12) { year++; month=1; }
			}
		}
		else if (minLevel==1) {
			day+=emStocksFileModel::GetDaysOfMonth(year,month);
			month++;
			if (month>12) { year++; month=1; }
		}
		else if (minLevel==2) {
			day+=365-28+emStocksFileModel::GetDaysOfMonth(year,2);
			year++;
		}
		else {
			day+=emStocksFileModel::GetDateDifference(year,1,1,year+10,1,1);
			year+=10;
		}
	}
}


void emStocksItemChart::PaintXScaleLabels(const emPainter & painter) const
{
	static const char * const monthTexts[] = {
		"January","February","March","April","May","June",
		"July","August","September","October","November","December"
	};
	char tmp[64];
	double textWidth[4],textHeight[4];
	double maxTextHeight,minTextHeight,fStartDay,fEndDay,x1,x2,y;
	int year,month,mday,day,startDay,endDay,level,maxLevel,y10;
	emColor c;

	maxTextHeight=GetMaxLabelHeight();
	minTextHeight=ViewToPanelDeltaY(6.0);

	textWidth[0]=   0.8*XFactor;
	textWidth[1]=  27.0*XFactor;
	textWidth[2]= 300.0*XFactor;
	textWidth[3]=3000.0*XFactor;

	textHeight[0]=emMin(maxTextHeight,textWidth[0]*0.8);
	textHeight[1]=emMin(maxTextHeight,textWidth[1]*0.2);
	textHeight[2]=emMin(maxTextHeight,textWidth[2]*0.4);
	textHeight[3]=emMin(maxTextHeight,textWidth[3]*0.4);

	if (textHeight[3]<minTextHeight) return;

	fStartDay=emMax(0.0,(ViewToPanelX(GetClipX1())-XOffset)/XFactor);
	fEndDay=emMin((double)TotalDays,(ViewToPanelX(GetClipX2())-XOffset)/XFactor);
	if (fStartDay>=fEndDay) return;
	startDay=(int)fStartDay;
	endDay=(int)fEndDay;

	y=emMax(
		emMin(YOffset+YFactor*LowerPrice,ViewToPanelY(GetClipY2())),
		YOffset+YFactor*UpperPrice+2.5*maxTextHeight
	);
	c=emColor(170,170,170,192);

	maxLevel=3;
	if (textHeight[2]>=0.9*textHeight[3]) {
		maxLevel=2;
		if (
			textHeight[1]>=0.9*textHeight[2] &&
			textWidth[1]/textHeight[1]>12.0
		) maxLevel=1;
	}

	for (level=maxLevel; level>=0; level--) {
		if (textHeight[level]<minTextHeight) break;
		y-=textHeight[level];

		year=StartYear;
		month=StartMonth;
		mday=StartDay;
		emStocksFileModel::AddDaysToDate(startDay,&year,&month,&mday);
		day=startDay;
		if (level>0) {
			if (mday>1) {
				day-=mday-1;
				mday=1;
			}
			if (level>1) {
				if (month>1) {
					day-=emStocksFileModel::GetDateDifference(year,1,1,year,month,1);
					month=1;
				}
				if (level>2) {
					if (year%10!=0) {
						y10=year-year%10;
						day-=emStocksFileModel::GetDateDifference(y10,1,1,year,1,1);
						year=y10;
					}
				}
			}
		}

		while (day<=endDay) {
			x1=XOffset+XFactor*emMax((double)day,fStartDay);

			if (level==0) {
				sprintf(tmp,"%d",mday);
				day++;
				mday++;
				if (mday>emStocksFileModel::GetDaysOfMonth(year,month)) {
					mday=1;
					month++;
					if (month>12) { year++; month=1; }
				}
			}
			else if (level==1) {
				if (maxLevel==1) sprintf(tmp,"%s %d",monthTexts[month-1],year);
				else strcpy(tmp,monthTexts[month-1]);
				day+=emStocksFileModel::GetDaysOfMonth(year,month);
				month++;
				if (month>12) { year++; month=1; }
			}
			else if (level==2) {
				sprintf(tmp,"%d",year);
				day+=365-28+emStocksFileModel::GetDaysOfMonth(year,2);
				year++;
			}
			else {
				sprintf(tmp,"%dx",year/10);
				day+=emStocksFileModel::GetDateDifference(year,1,1,year+10,1,1);
				year+=10;
			}

			x2=XOffset+XFactor*emMin((double)day,fEndDay);
			if (x1<x2) {
				painter.PaintTextBoxed(
					x1,y,x2-x1,textHeight[level],tmp,textHeight[level],c
				);
			}
		}
	}
}


void emStocksItemChart::PaintYScaleLines(const emPainter & painter) const
{
	double dist,minDist,nextDist,price,endPrice,maxThickness,x,y,w,f,t;
	int level,minLevel,maxLevel;
	emColor c;

	CalculateYScaleLevelRange(&minLevel,&minDist,&maxLevel);
	if (minLevel>maxLevel) return;

	maxThickness=emMin(0.002,ViewToPanelDeltaY(2.6));

	price=emMax(
		LowerPrice,
		(painter.GetUserClipY2()-YOffset+maxThickness*0.5)/YFactor
	);
	endPrice=emMin(
		UpperPrice,
		(painter.GetUserClipY1()-YOffset-maxThickness*0.5)/YFactor
	);
	if (price>endPrice) return;

	f=fmod(price,minDist);
	if (f>0.0) price+=minDist-f;
	else if (f<0.0) price-=f;

	x=XOffset;
	w=XFactor*TotalDays;
	c=emColor(128,128,128);

	for (; price<=endPrice; price+=minDist) {
		y=YOffset+YFactor*price;
		for (level=minLevel, dist=minDist; level<maxLevel; ) {
			nextDist=dist*((level&1)?2.0:5.0);
			f=price/nextDist;
			if (fabs(f-round(f))>0.001) break;
			dist=nextDist;
			level++;
		}
		t=dist*YFactor*(-0.01);
		if (level&1) t*=0.63;
		if (t>maxThickness) t=maxThickness;
		painter.PaintRect(x,y-t*0.5,w,t,c);
	}
}


void emStocksItemChart::PaintYScaleLabels(const emPainter & painter) const
{
	char frmt[32],tmp[128];
	double dist,minDist,nextDist,price,endPrice;
	double maxTextHeight,minTextHeight,x,xt,y,w,f,t;
	int level,minLevel,maxLevel;
	emColor c;

	CalculateYScaleLevelRange(&minLevel,&minDist,&maxLevel);
	if (minLevel>maxLevel) return;

	maxTextHeight=GetMaxLabelHeight();
	minTextHeight=ViewToPanelDeltaY(6.0);

	price=emMax(
		LowerPrice,
		(painter.GetUserClipY2()-YOffset+maxTextHeight)/YFactor
	);
	endPrice=emMin(
		UpperPrice,
		(painter.GetUserClipY1()-YOffset)/YFactor
	);
	if (price>endPrice) return;

	f=fmod(price,minDist);
	if (f>0.0) price+=minDist-f;
	else if (f<0.0) price-=f;

	x=XOffset;
	w=XFactor*TotalDays;
	c=emColor(170,170,170,192);
	xt=emMax(x,ViewToPanelX(GetClipX1()));

	for (; price<=endPrice; price+=minDist) {
		y=YOffset+YFactor*price;
		for (level=minLevel, dist=minDist; level<maxLevel; ) {
			nextDist=dist*((level&1)?2.0:5.0);
			f=price/nextDist;
			if (fabs(f-round(f))>0.001) break;
			dist=nextDist;
			level++;
		}
		t=dist*YFactor*(-0.16);
		if (level&1) t*=0.63;
		if (t<minTextHeight) continue;
		if (t>maxTextHeight) t=maxTextHeight;
		sprintf(frmt,"%%.%df",level>=0?0:((1-level)>>1));
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wformat-nonliteral"
		snprintf(tmp,sizeof(tmp),frmt,price);
#		pragma clang diagnostic pop
		tmp[sizeof(tmp)-1]=0;
		painter.PaintTextBoxed(xt,y-t,x+w-xt,t,tmp,t,c,0,EM_ALIGN_LEFT);
	}
}


void emStocksItemChart::CalculateYScaleLevelRange(
	int * pMinLevel, double * pMinDist, int * pMaxLevel
) const
{
	double minDist,maxDist,f;
	int minLevel,maxLevel;

	maxLevel=0;
	maxDist=1.0;

	f=(UpperPrice-LowerPrice)*0.4;
	while (maxDist>f) { maxLevel-=2; maxDist*=0.1; }
	while (maxDist*10.0<=f) { maxLevel+=2; maxDist*=10.0; }

	minLevel=maxLevel;
	minDist=maxDist;

	if (maxDist*5.0<=f) { maxLevel++; maxDist*=5.0; }

	f=emMax(
		emMax(fabs(LowerPrice),fabs(UpperPrice))*0.0001,
		ViewToPanelDeltaY(14.0)/(-YFactor)
	);

	while (minDist<f) { minLevel+=2; minDist*=10.0; }
	while (minDist*0.1>=f) { minLevel-=2; minDist*=0.1; }

	if (minDist*0.5>=f) { minLevel--; minDist*=0.5; }

	*pMinLevel=minLevel;
	*pMinDist=minDist;
	*pMaxLevel=maxLevel;
}


double emStocksItemChart::GetMaxLabelHeight() const
{
	return emMin(
		0.032,
		ViewToPanelDeltaY(
			emMax(10.0,(GetClipY2()-GetClipY1())*0.02)
		)
	);
}


void emStocksItemChart::PaintPriceBar(const emPainter & painter) const
{
	double x,xt,y,y1,y2,w,wt,textHeight,r;
	emColor c1,c2;

	if (!PriceOnSelectedDate || (!TradePrice && !DesiredPrice)) return;

	textHeight=(LowerPrice-UpperPrice)*YFactor*0.012;
	x=XOffset;
	w=XFactor*TotalDays;
	y1=YOffset+YFactor*(TradePrice?TradePrice.Value:DesiredPrice.Value);
	y2=YOffset+YFactor*PriceOnSelectedDate.Value;

	if (OwningShares) {
		if (y1>y2) c2=emColor(80,255,80,224);
		else       c2=emColor(255,80,80,224);
	}
	else {
		if (y1>y2) c2=emColor(255,80,255,224);
		else       c2=emColor(80,255,255,224);
	}
	c1=c2.GetBlended(emColor(128,128,255,224),50.0F);

	painter.PaintRect(
		x,emMin(y1,y2),w,fabs(y2-y1),
		emLinearGradientTexture(
			x,y1,c1.GetTransparented(30.0F),
			x,y2,c2.GetTransparented(10.0F)
		)
	);

	if (PanelToViewDeltaY(textHeight)<4.0) return;

	xt=XOffset+XFactor*(TotalDays-0.5);
	r=textHeight*0.12;
	painter.PaintEllipse(xt-r,y2-r,r*2,r*2,c2);

	wt=emPainter::GetTextSize(PriceOnSelectedDateText,textHeight);
	xt-=wt*0.5;
	if (xt>XOffset+XFactor*TotalDays-wt) xt=XOffset+XFactor*TotalDays-wt;
	if (y1>y2) y=y2-textHeight; else y=y2;
	painter.PaintTextBoxed(xt,y,wt,textHeight,PriceOnSelectedDateText,textHeight,c2);

	if (!TradePrice) return;

	if (TradeOffsetDays>=0) {
		xt=XOffset+XFactor*(TradeOffsetDays+0.5);
		if (TradeOffsetDays<TotalDays) {
			painter.PaintEllipse(xt-r,y1-r,r*2,r*2,c1);
		}
	}
	else if (TradeOffsetDays>INT_MIN) {
		xt=XOffset;
	}
	else {
		xt=XOffset+XFactor*TotalDays*0.5;
	}

	wt=emPainter::GetTextSize(TradePriceText,textHeight);
	xt-=wt*0.5;
	if (xt<XOffset) xt=XOffset;
	if (xt>XOffset+XFactor*TotalDays-wt) xt=XOffset+XFactor*TotalDays-wt;
	if (y1>y2) y=y1; else y=y1-textHeight;
	painter.PaintTextBoxed(xt,y,wt,textHeight,TradePriceText,textHeight,c1);
}


void emStocksItemChart::PaintDesiredPrice(const emPainter & painter) const
{
	double x,y,w,textHeight,thickness,v,v1,v2;
	emColor c;

	if (!DesiredPrice) return;

	thickness=emMax(
		ViewToPanelDeltaY(1.5),
		emMin((LowerPrice-UpperPrice)*YFactor*0.002,XFactor*0.5)
	);
	textHeight=(LowerPrice-UpperPrice)*YFactor*0.012;
	x=XOffset;
	w=XFactor*TotalDays;
	y=YOffset+YFactor*DesiredPrice.Value-thickness*0.5;
	c=emColor(255,255,0,224);

	painter.PaintRect(x,y,w,thickness,c);

	if (PanelToViewDeltaY(textHeight)<4.0) return;

	v=v1=v2=DesiredPrice.Value;
	if (PriceOnSelectedDate) {
		v1=v2=PriceOnSelectedDate.Value;
		if (TradePrice) {
			if (TradePrice.Value<v2) v1=TradePrice.Value;
			else                     v2=TradePrice.Value;
		}
	}
	if (v>v2 || (v>=v1 && v<(v1+v2)*0.5)) y-=textHeight;
	else y+=thickness;

	painter.PaintTextBoxed(
		x,y,w,textHeight,
		DesiredPriceText,textHeight,c,0,
		EM_ALIGN_RIGHT,EM_ALIGN_RIGHT
	);
}


void emStocksItemChart::PaintGraph(const emPainter & painter) const
{
	char tmp[64];
	double xOff,xFac,x1,y1,x2,y2,thickness,f,r;
	int i0,i1,i2,i3,i,n,year,month,mday,day;
	bool havePoints,haveTexts;
	emColor c1,c2;

	if (Prices.GetCount()<2) return;

	xOff=XOffset+XFactor*0.5;
	xFac=XFactor*(TotalDays-1)/(Prices.GetCount()-1);

	f=(painter.GetUserClipX1()-xOff)/xFac-0.5;
	if (f>=Prices.GetCount()) return;
	if (f<1.0) i1=0; else i1=(int)f;
	f=(painter.GetUserClipX2()-xOff)/xFac+0.5;
	if (f<=0.0) return;
	if (f>Prices.GetCount()-2) i2=Prices.GetCount()-1; else i2=(int)ceil(f);
	if (i1>=i2) return;

	thickness=emMax(
		ViewToPanelDeltaY(1.5),
		emMin((LowerPrice-UpperPrice)*YFactor*0.002,XFactor*0.1)
	);
	r=emMin(0.002,XFactor*0.1)*3.0;
	havePoints=(DaysPerPrice==1 && r>ViewToPanelDeltaY(1.2));
	haveTexts=(havePoints && r>ViewToPanelDeltaY(5.0));

	c1=emColor(255,255,255);
	c2=emColor(64,64,64);

	for (i0=i1; i0>0 && !Prices[i0].Valid; i0--);
	for (i3=i2; i3<Prices.GetCount()-1 && !Prices[i3].Valid; i3++);
	x1=0.0;
	y1=0.0;
	n=0;
	for (i=i0; i<=i3; i++) {
		if (!Prices[i].Valid) continue;
		x2=xOff+xFac*i;
		y2=YOffset+YFactor*Prices[i].Value;
		if (n) {
			painter.PaintLine(
				x1,y1,x2,y2,thickness,
				emRoundedStroke(c1),
				n>=2 || havePoints ? emStrokeEnd::BUTT : emStrokeEnd::CAP,
				havePoints ? emStrokeEnd::BUTT : emStrokeEnd::CAP
			);
		}
		x1=x2;
		y1=y2;
		n++;
	}

	if (!havePoints) return;

	for (i=i1; i<=i2; i++) {
		if (!Prices[i].Valid) continue;
		x1=xOff+xFac*i;
		y1=YOffset+YFactor*Prices[i].Value;
		painter.PaintEllipse(x1-r,y1-r,r*2,r*2,c1);
	}

	if (!haveTexts) return;

	year=StartYear;
	month=StartMonth;
	mday=StartDay;
	day=0;
	for (i=i1; i<=i2; i++) {
		if (!Prices[i].Valid) continue;
		x1=xOff+xFac*i;
		y1=YOffset+YFactor*Prices[i].Value;
		emStocksFileModel::AddDaysToDate(i-day,&year,&month,&mday);
		day=i;
		snprintf(tmp,sizeof(tmp),"%04d-%02d-%02d",year,month,mday);
		tmp[sizeof(tmp)-1]=0;
		painter.PaintTextBoxed(
			x1-r*0.8,y1-r*0.6,r*0.8*2,r*0.4,tmp,r,c2,c1,
			EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
		emStocksFileModel::SharePriceToString(
			Prices[i].Value,tmp,sizeof(tmp)
		);
		painter.PaintTextBoxed(
			x1-r*0.8,y1-r*0.2,r*0.8*2,r*0.9,tmp,r,c2,0,
			EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
	}
}


void emStocksItemChart::Price::Set(const char * str)
{
	const char * p;

	p=str;
	if (*p=='-') p++;
	if (*p=='.') p++;
	if (*p>='0' && *p<='9') {
		Valid=true;
		Value=atof(str);
	}
	else {
		Valid=false;
		Value=0.0;
	}
}
