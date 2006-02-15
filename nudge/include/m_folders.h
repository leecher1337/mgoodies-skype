/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_RELATIVE_FOLDERS_H
#define M_RELATIVE_FOLDERS_H

#define FOLDERS_API 301 //dunno why it's here but it is :)

#define PROFILE_PATH "%profile_path%"
#define CURRENT_PROFILE "%current_profile%"
#define MIRANDA_PATH "%miranda_path%"
#define PLUGINS_PATH "%miranda_path%" "\\plugins"


#define FOLDER_AVATARS                 PROFILE_PATH "\\" CURRENT_PROFILE "\\avatars"
#define FOLDER_VCARDS                  PROFILE_PATH "\\" CURRENT_PROFILE "\\vcards"
#define FOLDER_LOGS                    PROFILE_PATH "\\" CURRENT_PROFILE "\\logs"
#define FOLDER_RECEIVED_FILES          PROFILE_PATH "\\" CURRENT_PROFILE "\\received files"
#define FOLDER_DOCS                    MIRANDA_PATH "\\" "docs" 

#define FOLDER_CONFIG                  PLUGINS_PATH "\\" "config"


#define FOLDER_UPDATES                 MIRANDA_PATH "\\" "updates"

#define FOLDER_CUSTOMIZE               MIRANDA_PATH "\\" "customize"
#define FOLDER_CUSTOMIZE_SOUNDS        FOLDER_CUSTOMIZE "\\sounds"
#define FOLDER_CUSTOMIZE_ICONS         FOLDER_CUSTOMIZE "\\icons"
#define FOLDER_CUSTOMIZE_SMILEYS       FOLDER_CUSTOMIZE "\\smileys"
#define FOLDER_CUSTOMIZE_SKINS         FOLDER_CUSTOMIZE "\\skins"
#define FOLDER_CUSTOMIZE_THEMES        FOLDER_CUSTOMIZE "\\themes"


#define FOLDERS_NAME_MAX_SIZE 64  //maximum name and section size

typedef struct{
  int cbSize;                                  //size of struct
  char szSection[FOLDERS_NAME_MAX_SIZE];       //section name, if it doesn't exist it will be created otherwise it will just add this entry to it
  char szName[FOLDERS_NAME_MAX_SIZE];          //entry name - will be shown in options
} FOLDERSDATA;

/*Folders/Register/Path service
  wParam - (LPARAM) (char *). Default string format. This is the fallback
  string in case there's no entry in the database for this folder. This should
  be the intial value for the path, users will be able to change it later.
  String is strdup()'d so you can free it after the call.
  lParam - (WPARAM) (const FOLDERDATA *) - Data structure filled with 
  the necessary information.
  Returns a handle to the registered path or 0 on error. 
  You need to use this to call the other services.
*/
#define MS_FOLDERS_REGISTER_PATH "Folders/Register/Path"

/*Folders/Get/PathSize service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (int *) - pointer to the variable that receives the size of the path 
  string (not including the null character).
  Returns the size of the buffer.
*/
#define MS_FOLDERS_GET_SIZE "Folders/Get/PathSize"

/*Folders/Get/Path service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (char *) pointer to the buffer that receives the path without the last \
  It must be big enough !!! - use MS_FOLDERS_GETPATH_SIZE to find out the size.
  Should return 0 on success, or nonzero otherwise. Currently it only returns 0.
*/
#define MS_FOLDERS_GET_PATH "Folders/Get/Path"

/*Folders/GetRelativePath/Alloc service
  wParam - (WPARAM) (int) - Handle to registered path
  lParam - (LPARAM) (char **) - address of a string variable (char *) where the path should be stored
  (the last \ won't be copied).
  This service is the same as MS_FOLDERS_GET_PATH with the difference that this service
  allocates the needed space for the buffer. It uses miranda's memory functions for that and you need
  to use those to free the resulting buffer.
  Should return 0 on success, or nonzero otherwise. Currently it only returns 0.
*/
#define MS_FOLDERS_GET_PATH_ALLOC "Folders/Get/Path/Alloc"

typedef struct{
  int hRegisteredPath;      //handle of registered path
  const char *szAppendData; //string data to append to path
} FOLDERSAPPENDDATA;

/*Folders/Get/Path/Append service
  wParam - (WPARAM) (const FOLDERAPPENDDATA *) data
  lParam - (LPARAM) (char *) string
  This service behaves exactly as MS_FOLDERS_GET_PATH but
  it will also append szAppendData to the path before returning.
*/
#define MS_FOLDERS_GET_PATH_APPEND "Folders/Get/Path/Append"

/*Folders/Get/Path/Alloc/Append service
  wParam - (WPARAM) (const FOLDERAPPENDDATA *) data
  lParam - (LPARAM) (char **) pointer to string
  This service behaves exactly as MS_FOLDERS_GET_PATH_ALLOC but
  it will also append szAppendData to the path before returning.
*/
#define MS_FOLDERS_GET_PATH_ALLOC_APPEND "Folders/Get/Path/Alloc/Append"

#endif //M_RELATIVE_FOLDERS_H