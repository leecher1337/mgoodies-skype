/*  phonetic.c - generic replacement aglogithms for phonetic transformation
    Copyright (C) 2000 Björn Jacke

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License version 2.1 as published by the Free Software Foundation;
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Björn Jacke may be reached by email at bjoern.jacke@gmx.de

    Changelog:

    2000-01-05  Björn Jacke <bjoern.jacke AT gmx.de>
                Initial Release insprired by the article about phonetic
                transformations out of c't 25/1999

    2007-07-20  Björn Jacke <bjoern.jacke AT gmx.de>
		Released under MPL/GPL/LGPL tri-license for Hunspell
		
    2007-08-22  LÃ¡szlÃ³ NÃ©meth <nemeth at OOo>
                Porting from Aspell to Hunspell by little modifications
*/

#ifndef __PHONETHXX__
#define __PHONETHXX__

#define PHONETABLE_HASH_SIZE 256
struct phonetable {
  char utf8;
  cs_info * lang;
  int num;
  const char * * rules;
  int hash[PHONETABLE_HASH_SIZE];
};

void init_phonet_hash(phonetable & parms);

int phonet (const char * inword, char * target,
              int len, phonetable & phone);

#endif
