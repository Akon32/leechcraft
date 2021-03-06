/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "bytearray.h"
#include <QString>

using namespace LeechCraft;

ByteArray::ByteArray (QObject *parent)
: QObject (parent)
, Imp_ (new QByteArray ())
{
}

ByteArray::ByteArray (const ByteArray& obj, QObject *parent)
: QObject (parent)
, Imp_ (new QByteArray (*obj.Imp_))
{
}

ByteArray::ByteArray (const QByteArray& obj, QObject* parent)
: QObject (parent)
, Imp_ (new QByteArray (obj))
{
}

ByteArray::~ByteArray ()
{
}

ByteArray& ByteArray::operator= (const ByteArray& ba)
{
	*Imp_ = *ba.Imp_;
	setParent (ba.parent ());
	return *this;
}

bool ByteArray::operator!= (const QString& str) const
{
	return Imp_->operator!= (str);
}

ByteArray& ByteArray::operator+= (const ByteArray& ba)
{
	*Imp_ += *ba.Imp_;
	return *this;
}

ByteArray& ByteArray::operator+= (const QString& str)
{
	*Imp_ += str;
	return *this;
}

ByteArray& ByteArray::operator+= (char c)
{
	*Imp_ += c;
	return *this;
}

bool ByteArray::operator< (const QString& str) const
{
	return *Imp_ < str;
}

bool ByteArray::operator<= (const QString& str) const
{
	return *Imp_ <= str;
}

bool ByteArray::operator== (const QString& str) const
{
	return *Imp_ == str;
}

bool ByteArray::operator> (const QString& str) const
{
	return *Imp_ > str;
}

bool ByteArray::operator>= (const QString& str) const
{
	return *Imp_ >= str;
}

ByteArray::operator QByteArray () const
{
	return QByteArray (*Imp_);
}

ByteArray& ByteArray::append (const ByteArray& ba)
{
	Imp_->append (*ba.Imp_);
	return *this;
}

ByteArray& ByteArray::append (const QString& str)
{
	Imp_->append (str);
	return *this;
}

ByteArray& ByteArray::append (char ch)
{
	Imp_->append (ch);
	return *this;
}

char ByteArray::at (int i) const
{
	return Imp_->at (i);
}

int ByteArray::capacity () const
{
	return Imp_->capacity ();
}

void ByteArray::chop (int i)
{
	Imp_->chop (i);
}

void ByteArray::clear ()
{
	Imp_->clear ();
}

bool ByteArray::contains (const ByteArray& ba) const
{
	return Imp_->contains (*ba.Imp_);
}

bool ByteArray::contains (char c) const
{
	return Imp_->contains (c);
}

int ByteArray::count (const ByteArray& ba) const
{
	return Imp_->count (*ba.Imp_);
}

int ByteArray::count (char c) const
{
	return Imp_->count (c);
}

int ByteArray::count () const
{
	return Imp_->count ();
}

bool ByteArray::endsWith (const ByteArray& ba) const
{
	return Imp_->endsWith (*ba.Imp_);
}

bool ByteArray::endsWith (char c) const
{
	return Imp_->endsWith (c);
}

ByteArray& ByteArray::fill (char c, int size)
{
	Imp_->fill (c, size);
	return *this;
}

int ByteArray::indexOf (const ByteArray& ba, int from) const
{
	return Imp_->indexOf (*ba.Imp_, from);
}

int ByteArray::indexOf (const QString& str, int from) const
{
	return Imp_->indexOf (str, from);
}

int ByteArray::indexOf (char c, int from) const
{
	return Imp_->indexOf (c, from);
}

ByteArray& ByteArray::insert (int i, const ByteArray& ba)
{
	Imp_->insert (i, *ba.Imp_);
	return *this;
}

ByteArray& ByteArray::insert (int i, const QString& str)
{
	Imp_->insert (i, str);
	return *this;
}

ByteArray& ByteArray::insert (int i, char c)
{
	Imp_->insert (i, c);
	return *this;
}

bool ByteArray::isEmpty () const
{
	return Imp_->isEmpty ();
}

bool ByteArray::isNull () const
{
	return Imp_->isNull ();
}

int ByteArray::lastIndexOf (const ByteArray& ba, int from) const
{
	return Imp_->lastIndexOf (*ba.Imp_, from);
}

int ByteArray::lastIndexOf (const QString& str, int from) const
{
	return Imp_->lastIndexOf (str, from);
}

int ByteArray::lastIndexOf (char c, int from) const
{
	return Imp_->lastIndexOf (c, from);
}

ByteArray ByteArray::left (int pos) const
{
	return ByteArray (Imp_->left (pos), parent ());
}

ByteArray ByteArray::leftJustified (int width, char fill, bool truncate) const
{
	return ByteArray (Imp_->leftJustified (width, fill, truncate),
			parent ());
}

int ByteArray::length () const
{
	return Imp_->length ();
}

ByteArray ByteArray::mid (int pos, int len) const
{
	return ByteArray (Imp_->mid (pos, len), parent ());
}

ByteArray& ByteArray::prepend (const ByteArray& ba)
{
	Imp_->prepend (*ba.Imp_);
	return *this;
}

ByteArray& ByteArray::prepend (char c)
{
	Imp_->prepend (c);
	return *this;
}

void ByteArray::push_back (const ByteArray& ba)
{
	Imp_->push_back (*ba.Imp_);
}

void ByteArray::push_back (char c)
{
	Imp_->push_back (c);
}

void ByteArray::push_front (const ByteArray& ba)
{
	Imp_->push_front (*ba.Imp_);
}

void ByteArray::push_front (char c)
{
	Imp_->push_front (c);
}

ByteArray& ByteArray::remove (int pos, int len)
{
	Imp_->remove (pos, len);
	return *this;
}

ByteArray& ByteArray::replace (int pos, int len, const ByteArray& after)
{
	Imp_->replace (pos, len, *after.Imp_);
	return *this;
}

ByteArray& ByteArray::replace (const ByteArray& before, const ByteArray& after)
{
	Imp_->replace (*before.Imp_, *after.Imp_);
	return *this;
}

ByteArray& ByteArray::replace (const QString& str, const ByteArray& after)
{
	Imp_->replace (str, *after.Imp_);
	return *this;
}

ByteArray& ByteArray::replace (char c, const ByteArray& after)
{
	Imp_->replace (c, *after.Imp_);
	return *this;
}

ByteArray& ByteArray::replace (char c, const QString& str)
{
	Imp_->replace (c, str);
	return *this;
}

ByteArray& ByteArray::replace (char c, char after)
{
	Imp_->replace (c, after);
	return *this;
}

void ByteArray::reserve (int size)
{
	Imp_->reserve (size);
}

void ByteArray::resize (int size)
{
	Imp_->resize (size);
}

ByteArray ByteArray::right (int len) const
{
	return ByteArray (Imp_->right (len), parent ());
}

ByteArray ByteArray::rightJustified (int width, char fill, bool truncate) const
{
	return ByteArray (Imp_->rightJustified (width, fill, truncate), parent ());
}

ByteArray& ByteArray::setNum (int n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (uint n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (short n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (ushort n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (qlonglong n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (qulonglong n, int base)
{
	Imp_->setNum (n, base);
	return *this;
}

ByteArray& ByteArray::setNum (double n, char f, int prec)
{
	Imp_->setNum (n, f, prec);
	return *this;
}

ByteArray& ByteArray::setNum (float n, char f, int prec)
{
	Imp_->setNum (n, f, prec);
	return *this;
}

ByteArray ByteArray::simplified () const
{
	return ByteArray (Imp_->simplified (), parent ());
}

int ByteArray::size () const
{
	return Imp_->size ();
}

QList<ByteArray> ByteArray::split (char sep) const
{
	QList<QByteArray> splitted = Imp_->split (sep);
	QList<ByteArray> result;
	for (QList<QByteArray>::const_iterator i = splitted.begin (),
			end = splitted.end (); i != end; ++i)
		result.push_back (ByteArray (*i, parent ()));
	return result;
}

void ByteArray::squeeze ()
{
	Imp_->squeeze ();
}

bool ByteArray::startsWith (const ByteArray& ba) const
{
	return Imp_->startsWith (*ba.Imp_);
}

bool ByteArray::startsWith (char c) const
{
	return Imp_->startsWith (c);
}

ByteArray ByteArray::toBase64 () const
{
	return ByteArray (Imp_->toBase64 (), parent ());
}

double ByteArray::toDouble () const
{
	return Imp_->toDouble ();
}

float ByteArray::toFloat () const
{
	return Imp_->toFloat ();
}

ByteArray ByteArray::toHex () const
{
	return ByteArray (Imp_->toHex (), parent ());
}

int ByteArray::toInt (int base) const
{
	return Imp_->toInt (0, base);
}

long ByteArray::toLong (int base) const
{
	return Imp_->toLong (0, base);
}

qlonglong ByteArray::toLongLong (int base) const
{
	return Imp_->toLongLong (0, base);
}

ByteArray ByteArray::toLower () const
{
	return ByteArray (Imp_->toLower (), parent ());
}

ByteArray ByteArray::toPercentEncoding (const ByteArray& ex,
		const ByteArray& in, char p) const
{
	return ByteArray (Imp_->toPercentEncoding (*ex.Imp_, *in.Imp_, p),
			parent ());
}

short ByteArray::toShort (int base) const
{
	return Imp_->toShort (0, base);
}

uint ByteArray::toUInt (int base) const
{
	return Imp_->toUInt (0, base);
}

ulong ByteArray::toULong (int base) const
{
	return Imp_->toULong (0, base);
}

qulonglong ByteArray::toULongLong (int base) const
{
	return Imp_->toULongLong (0, base);
}

ushort ByteArray::toUShort (int base) const
{
	return Imp_->toUShort (0, base);
}

ByteArray ByteArray::toUpper () const
{
	return Imp_->toUpper ();
}

ByteArray ByteArray::trimmed () const
{
	return Imp_->trimmed ();
}

void ByteArray::truncate (int pos)
{
	Imp_->truncate (pos);
}

