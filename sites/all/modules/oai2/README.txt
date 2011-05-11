//$Id
  oai2.module

  Copyright (C) 2006  Ron Jerome

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


Description:
============
This module provides an implementation the OAI V2 Data-Provider for the Drupal 
Bibliography module.  

Requirements:
=============
Drupal 6.x, bibliography module.

Installation:
=============
Unpack the oai2 package in your sites/all/modules directory.  Visit admin/build/modules
and enable the OAI2 module, the module will report success or failure of the 
installation.  

You should now be able to visit example.com/oai?verb=Identify and get some basic
information about your repository.


Acknowledgements:
=================
This module was inspired by and I have used some code from http://physnet.uni-oldenburg.de/oai/
writen by Heinrich Stamerjohanns.
