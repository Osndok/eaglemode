//------------------------------------------------------------------------------
// emStocksRec.cpp
//
// Copyright (C) 2021-2022 Oliver Hamann.
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

#include <emStocks/emStocksRec.h>
#include <emCore/emAvlTreeSet.h>


emStocksRec::emStocksRec()
	: emStructRec(),
	Stocks(this,"Stocks",0,INT_MAX)
{
}


emStocksRec::~emStocksRec()
{
}


const char * emStocksRec::GetFormatName() const
{
	return "emStocks";
}


emStocksRec::StockRec::StockRec()
	: emStructRec(),
	Id(this,"Id"),
	Name(this,"Name"),
	Symbol(this,"Symbol"),
	WKN(this,"WKN"),
	ISIN(this,"ISIN"),
	Country(this,"Country"),
	Sector(this,"Sector"),
	Collection(this,"Collection"),
	Comment(this,"Comment"),
	OwningShares(this,"OwningShares"),
	OwnShares(this,"OwnShares"),
	TradePrice(this,"TradePrice"),
	TradeDate(this,"TradeDate"),
	Prices(this,"Prices"),
	LastPriceDate(this,"LastPriceDate"),
	DesiredPrice(this,"DesiredPrice"),
	ExpectedDividend(this,"ExpectedDividend"),
	InquiryDate(this,"InquiryDate"),
	Interest(
		this,"Interest",
		MEDIUM_INTEREST,
		"LOW_INTEREST",
		"MEDIUM_INTEREST",
		"HIGH_INTEREST",
		NULL
	),
	WebPages(this,"WebPages")
{
}


emStocksRec::StockRec::~StockRec()
{
}


const char * emStocksRec::StockRec::GetPricePtrOfDate(
	const char * date
) const
{
	const char * p, * q;
	int d;
	bool datesValid;

	d=GetDateDifference(date,LastPriceDate.Get(),&datesValid);
	if (!datesValid || d<0) return "";
	p=Prices.Get().Get();
	for (q=p+strlen(p); q>p; q--) {
		while (q>p && q[-1]!='|') q--;
		d--;
		if (d<0) return q;
	}
	return "";
}


emString emStocksRec::StockRec::GetPriceOfDate(
	const char * date
) const
{
	const char * p, * q;

	p=GetPricePtrOfDate(date);
	q=p;
	while (*q && *q!='|') q++;
	return emString(p,q-p);
}


emString emStocksRec::StockRec::GetPricesDateBefore(
	const char * date
) const
{
	const char * p, * q;
	int d,n;

	d=GetDateDifference(date,LastPriceDate.Get());
	p=Prices.Get().Get();
	for (q=p+strlen(p), n=0; q>p; q--, n++) {
		while (q>p && q[-1]!='|') q--;
		if (n>d && *q && *q!='|') return AddDaysToDate(-n,LastPriceDate.Get());
	}
	return emString();
}


emString emStocksRec::StockRec::GetPricesDateAfter(
	const char * date
) const
{
	const char * p, * q;
	int d,n,m;

	d=GetDateDifference(date,LastPriceDate.Get());
	if (d<=0) return emString();
	p=Prices.Get().Get();
	for (q=p+strlen(p), n=0, m=-1; q>p; q--, n++) {
		while (q>p && q[-1]!='|') q--;
		if (*q && *q!='|') m=n;
		if (n+1>=d) break;
	}
	if (m>=0) return  AddDaysToDate(-m,LastPriceDate.Get());
	return emString();
}


void emStocksRec::StockRec::AddPrice(
	const char * date, const char * price
)
{
	emString prices;
	const char * p, * q, * e;
	int n,i,j;

	prices=Prices.Get();

	n=0;
	p=prices.Get();
	if (*p) {
		n=1;
		do {
			if (*p=='|') n++;
			p++;
		} while (*p);
	}

	if (n<=0) {
		Prices=price;
		LastPriceDate=date;
		return;
	}

	i=n-1+GetDateDifference(LastPriceDate.Get(),date);

	if (i>=n) {
		p=prices.Get();
		while (*p && (i+1>MAX_NUM_PRICES || *p=='|')) {
			do { p++; } while (*p && p[-1]!='|');
			n--;
			i--;
		}
		if (n<=0) {
			Prices=price;
			LastPriceDate=date;
			return;
		}
		if (p>prices.Get()) prices.Remove(0,p-prices.Get());
	}

	if (i<0) {
		p=prices.Get();
		e=p+strlen(p);
		q=e;
		while (q>p && (-i+n>MAX_NUM_PRICES || q[-1]=='|')) {
			do { q--; } while (q>p && *q!='|');
			n--;
			LastPriceDate=AddDaysToDate(-1,LastPriceDate.Get());
		}
		if (n<=0) {
			Prices=price;
			LastPriceDate=date;
			return;
		}
		if (q<e) prices.Remove(q-prices.Get(),e-q);
	}

	if (i>=n) {
		prices.Add('|',i+1-n);
		n=i+1;
		LastPriceDate=date;
	}

	if (i<0) {
		prices.Insert(0,'|',-i);
		n+=-i;
		i=0;
	}

	p=prices.Get();
	e=p+strlen(p);
	for (j=n-1; ; j--) {
		for (q=e; q>p && q[-1]!='|'; q--);
		if (j<=i) break;
		e=q-1;
	}
	prices.Replace(q-p,e-q,price);
	Prices=prices;
}


bool emStocksRec::StockRec::IsMatchingSearchText(
	const char * searchText
) const
{
	struct Helper {
		const char * SearchText;
		int SearchTextLen;

		Helper(const char * searchText)
			: SearchText(searchText),
			SearchTextLen(strlen(SearchText))
		{
		}

		bool operator()(const char * str)
		{
			int i,l;

			l=strlen(str);
			for (i=0; i<=l-SearchTextLen; i++) {
				if (strncasecmp(str+i,SearchText,SearchTextLen)==0) return true;
			}
			return false;
		}
	};

	Helper h(searchText);

	if (h(Name.Get())) return true;
	if (h(Symbol.Get())) return true;
	if (h(WKN.Get())) return true;
	if (h(ISIN.Get())) return true;
	if (h(Country.Get())) return true;
	if (h(Sector.Get())) return true;
	if (h(Collection.Get())) return true;
	if (h(Comment.Get())) return true;
	return false;
}


bool emStocksRec::StockRec::GetTradeValue(double * pResult) const
{
	if (
		!OwningShares.Get() ||
		TradePrice.Get().IsEmpty() ||
		OwnShares.Get().IsEmpty()
	) {
		*pResult=0.0;
		return false;
	}
	*pResult=atof(TradePrice.Get())*atof(OwnShares.Get());
	return true;
}


bool emStocksRec::StockRec::GetValueOfDate(
	double * pResult, const char * date
) const
{
	const char * price;

	if (
		!OwningShares.Get() ||
		OwnShares.Get().IsEmpty()
	) {
		*pResult=0.0;
		return false;
	}
	price = GetPricePtrOfDate(date);
	if (*price<'0' || *price>'9') {
		*pResult=0.0;
		return false;
	}

	*pResult=atof(price)*atof(OwnShares.Get());
	return true;
}


bool emStocksRec::StockRec::GetDifferenceValueOfDate(
	double * pResult, const char * date
) const
{
	double v1,v2;

	if (!GetTradeValue(&v1) || !GetValueOfDate(&v2,date)) {
		*pResult=0.0;
		return false;
	}
	*pResult=v2-v1;
	return true;
}


bool emStocksRec::StockRec::GetAchievementOfDate(
	double * pResult, const char * date, bool relative
) const
{
	const char * price;
	double d,p,t;

	if (DesiredPrice.Get().IsEmpty()) {
		*pResult=0.0;
		return false;
	}
	d=atof(DesiredPrice.Get());
	if (d<1E-10) {
		*pResult=0.0;
		return false;
	}

	price = GetPricePtrOfDate(date);
	if (*price<'0' || *price>'9') {
		*pResult=0.0;
		return false;
	}
	p=atof(price);
	if (p<1E-10) {
		*pResult=0.0;
		return false;
	}

	if (relative) {
		if (TradePrice.Get().IsEmpty()) {
			*pResult=0.0;
			return false;
		}
		t=atof(TradePrice.Get());
		if (t<1E-10) {
			*pResult=0.0;
			return false;
		}

		if (fabs(d-t)<1E-10) d=t+(OwningShares.Get()?1E-10:-1E-10);
		d=(p-t)/(d-t);
	}
	else {
		if (OwningShares.Get()) d=p/d; else d=d/p;
	}

	*pResult=d*100.0;
	return true;
}


bool emStocksRec::StockRec::GetRiseUntilDate(
	double * pResult, const char * date, int days
) const
{
	const char * p, * q, * r;
	double m,c;
	int d,d1,d2,n;

	q=GetPricePtrOfDate(date);
	if (*q<'0' || *q>'9') {
		*pResult=0.0;
		return false;
	}
	c=atof(q);
	if (c<1E-10) {
		*pResult=0.0;
		return false;
	}

	p=Prices.Get().Get();
	r=q;
	d1=days-days/6;
	d2=days+days/6;
	m=0.0;
	n=0;
	for (d=1; q>p && d<=d2; q--, d++) {
		while (q>p && q[-1]!='|') q--;
		if (*q<'0' || *q>'9') continue;
		r=q;
		if (d<d1) continue;
		m+=atof(q);
		n++;
	}

	if (n==0) m=atof(r);
	else m/=n;
	if (m<1E-10) {
		*pResult=0.0;
		return false;
	}

	if (OwningShares.Get()) c=c/m; else c=m/c;

	*pResult=c*100.0;
	return true;
}


emString emStocksRec::InventStockId() const
{
	emAvlTreeSet<int> ids;
	int i,id;

	for (id=0, i=0; i<Stocks.GetCount(); i++) {
		id=emMax(id,atoi(Stocks[i].Id.Get()));
	}
	if (id<INT_MAX) {
		id++;
	}
	else {
		for (i=0; i<Stocks.GetCount(); i++) {
			ids.Insert(atoi(Stocks[i].Id.Get()));
		}
		for (id=1; ids.Contains(id); id++);
	}
	return emString::Format("%d",id);
}


int emStocksRec::GetStockIndexById(const char * id) const
{
	int i;

	for (i=Stocks.GetCount()-1; i>=0; i--) {
		if (Stocks[i].Id.Get()==id) break;
	}
	return i;
}


int emStocksRec::GetStockIndexByStock(
	const emStocksRec::StockRec * stockRec
) const
{
	int i;

	for (i=Stocks.GetCount()-1; i>=0; i--) {
		if (&Stocks[i]==stockRec) break;
	}
	return i;
}


bool emStocksRec::ParseDate(
	const char * date, int * pYear, int * pMonth, int * pDay
)
{
	int s,y,m,d;

	s=1;
	y=m=d=0;
	while ((*date<'0' || *date>'9') && *date) {
		if (*date=='-') s=-1;
		date++;
	}
	while (*date>='0' && *date<='9') y=y*10+(*date++ - '0');
	while ((*date<'0' || *date>'9') && *date) date++;
	while (*date>='0' && *date<='9') m=m*10+(*date++ - '0');
	while ((*date<'0' || *date>'9') && *date) date++;
	while (*date>='0' && *date<='9') d=d*10+(*date++ - '0');
	if (pYear) *pYear=s*y;
	if (pMonth) *pMonth=m;
	if (pDay) *pDay=d;
	return m>=1 && d>=1;
}


int emStocksRec::CompareDates(const char * date1, const char * date2)
{
	int y1,m1,d1,y2,m2,d2;

	ParseDate(date1,&y1,&m1,&d1);
	ParseDate(date2,&y2,&m2,&d2);
	return ((y1-y2)*16+m1-m2)*32+d1-d2;
}


int emStocksRec::GetDaysOfMonth(int year, int month)
{
	switch (month) {
		case 2:
			if (year%4==0 && (year%100!=0 || year%400==0)) return 29;
			return 28;
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
		default:
			return 31;
	}
}


void emStocksRec::AddDaysToDate(
	int days, int * pYear, int * pMonth, int * pDay
)
{
	int y,m,d,n;

	y=*pYear;
	m=*pMonth;
	d=*pDay;

	d+=days;
	while (d<-213) {
		d+=365-28+GetDaysOfMonth(m>2?y:y-1,2);
		y--;
	}
	while (d>243) {
		y++;
		d-=365-28+GetDaysOfMonth(m>2?y:y-1,2);
	}
	while (d<1) {
		m--;
		if (m<1) { y--; m=12; }
		d+=GetDaysOfMonth(y,m);
	}
	while (d>28) {
		n=GetDaysOfMonth(y,m);
		if (d<=n) break;
		d-=n;
		m++;
		if (m>12) { y++; m=1; }
	}

	*pYear=y;
	*pMonth=m;
	*pDay=d;
}


emString emStocksRec::AddDaysToDate(int days, const char * date)
{
	int y,m,d;

	ParseDate(date,&y,&m,&d);
	AddDaysToDate(days,&y,&m,&d);
	return emString::Format("%04d-%02d-%02d",y,m,d);
}


int emStocksRec::GetDateDifference(
	int y1, int m1, int d1, int y2, int m2, int d2
)
{
	int days;

	days=d2-d1;
	if (y1!=y2) {
		days+=(y2-y1)*365+(m2-m1)*30;
		AddDaysToDate(days,&y1,&m1,&d1);
		days+=d2-d1;
	}
	while (y1<y2 || (y1==y2 && m1<m2)) {
		days+=GetDaysOfMonth(y1,m1);
		m1++; if (m1>12) { y1++; m1=1; }
	}
	while (y1>y2 || (y1==y2 && m1>m2)) {
		days-=GetDaysOfMonth(y2,m2);
		m2++; if (m2>12) { y2++; m2=1; }
	}
	return days;
}


int emStocksRec::GetDateDifference(
	const char * fromDate, const char * toDate, bool * pDatesValid
)
{
	int y1,m1,d1,y2,m2,d2;
	bool fromValid,toValid;

	fromValid=ParseDate(fromDate,&y1,&m1,&d1);
	toValid  =ParseDate(toDate  ,&y2,&m2,&d2);
	if (pDatesValid) {
		*pDatesValid=(fromValid && toValid);
	}
	return GetDateDifference(y1,m1,d1,y2,m2,d2);
}


emString emStocksRec::GetCurrentDate()
{
	time_t t;
	struct tm tmbuf;
	struct tm * p;

	t=time(NULL);
	p=localtime_r(&t,&tmbuf);
	if (!p) return "0000-00-00";
	return emString::Format(
		"%04d-%02d-%02d",
		(int)p->tm_year+1900,
		(int)p->tm_mon+1,
		(int)p->tm_mday
	);
}


emString emStocksRec::GetLatestPricesDate() const
{
	int i,j;

	j=-1;
	for (i=0; i<Stocks.GetCount(); i++) {
		if (Stocks[i].LastPriceDate.Get().IsEmpty()) continue;
		if (
			j<0 ||
			CompareDates(
				Stocks[i].LastPriceDate.Get(),
				Stocks[j].LastPriceDate.Get()
			)>0
		) j=i;
	}
	return j>=0 ? Stocks[j].LastPriceDate.Get() : emString();
}


emString emStocksRec::GetPricesDateBefore(const char * date) const
{
	emString bestDate,stockDate;
	int i;

	for (i=0; i<Stocks.GetCount(); i++) {
		stockDate=Stocks[i].GetPricesDateBefore(date);
		if (stockDate.IsEmpty()) continue;
		if (bestDate.IsEmpty() || CompareDates(bestDate,stockDate)<0) {
			bestDate=stockDate;
		}
	}
	return bestDate;
}


emString emStocksRec::GetPricesDateAfter(const char * date) const
{
	emString bestDate,stockDate;
	int i;

	for (i=0; i<Stocks.GetCount(); i++) {
		stockDate=Stocks[i].GetPricesDateAfter(date);
		if (stockDate.IsEmpty()) continue;
		if (bestDate.IsEmpty() || CompareDates(bestDate,stockDate)>0) {
			bestDate=stockDate;
		}
	}
	return bestDate;
}


void emStocksRec::SharePriceToString(
	double price, char * buf, int bufSize
)
{
	char frmt[32];
	double m;
	int d;

	for (d=0, m=1000.0; ; d++, m/=10.0) {
		if (fabs(price)>=m) break;
		if (d>=8) {
			if (price==0.0) d=0;
			break;
		}
	}
	sprintf(frmt,"%%.%df",d);
	snprintf(buf,bufSize,frmt,price);
	buf[bufSize-1]=0;
}


emString emStocksRec::SharePriceToString(double price)
{
	char tmp[64];

	SharePriceToString(price,tmp,sizeof(tmp));
	return emString(tmp);
}


void emStocksRec::PaymentPriceToString(
	double price, char * buf, int bufSize
)
{
	snprintf(buf,bufSize,"%.2f",price);
	buf[bufSize-1]=0;
}


emString emStocksRec::PaymentPriceToString(double price)
{
	char tmp[64];

	PaymentPriceToString(price,tmp,sizeof(tmp));
	return emString(tmp);
}
