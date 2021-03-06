/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#include "../private.h"

#ifdef _WIN32
#define NOMINMAX
#include <direct.h>
#include <io.h>
#include <windows.h>

#endif
namespace Axon {

	void PathUtil::splitPath(const String& path, String& dir, String& fname, String& ext) {
		dir = path;

		fname.clear();
		ext.clear();

		size_t slen = dir.length();
		size_t len = slen;

		// find extension
		for (; len>0; len--) {
			if (dir[len-1] == L'.') {
				ext = dir.c_str() + len - 1;
				dir.resize(len-1);
				break;
			}
		}

		// find file name;
		if (len == 0) len = slen;
		for (; len>0; len--) {
			if (isDirectoryLetter(dir[len-1])) {
				fname = dir.c_str() + len;
				dir.resize(len);
				break;
			}
		}

		return;
	}

	String PathUtil::removeExt(const String& path) {
		String out = path;

		size_t len = out.length();

		// find extension
		for (int i = (int)len-1; i>=0; i--) {
			if (out[i] == L'.') {
				out.resize(i);
				break;
			}
		}

		return out;
	}

	String PathUtil::removeDir(const String& fullpath) {
		for (int i = (int)fullpath.size()-1; i>=0; i--) {
			if (isDirectoryLetter(fullpath[i])) {
				return fullpath.c_str() + i + 1;
			}
		}

		return fullpath;
	}

	String PathUtil::removeDirOne(const String& path)
	{
		for (size_t i = 0; i < path.size(); i++) {
			if (isDirectoryLetter(path[i])) {
				return path.c_str() + i + 1;
			}
		}
		return path;
	}


	String PathUtil::getDir(const String& fullpath) {
		String out = fullpath;
		intptr_t i;

		size_t len = out.length();
		for (i = len - 1; i >= 0; i--) {
			if (isDirectoryLetter(out[i])) {
				break;
			}
		}

		// we need / or \ char, so + 1
		out.resize(i + 1);

		return out;
	}

	bool PathUtil::haveDir(const String& fullpath) {
		for (size_t i = 0; i < fullpath.length(); i++)
			if (isDirectoryLetter(fullpath[i]))
				return true;

		return false;
	}


	String PathUtil::getExt(const String& fullpath) {
		size_t len = fullpath.length();

		// find extension
		for (; len>0; len--) {
			if (fullpath[len-1] == L'.') {
				return fullpath.c_str() + len - 1;
			}
		}

		return String();
	}

	String PathUtil::getName(const String& fullpath) {
		size_t extpos = fullpath.rfind('.');
		size_t pathpos = fullpath.find_last_of("/\\");

		if (pathpos == String::npos) {
			pathpos = 0;
		} else {
			pathpos++;
		}

		if (extpos == String::npos || extpos <= pathpos ) {
			extpos = fullpath.size();
		}

		return fullpath.substr(pathpos, extpos-pathpos);
	}

	bool PathUtil::createDir(const String& OSPath) {
		if (OSPath.find("..") != String::npos  || OSPath.find("::") != String::npos) {
			return false;
		}

		String path = removeFilename(OSPath);

		if (OsUtil::mkdir(path.c_str()) == 0)
			return true;

		return false;
	}

	String PathUtil::normalizePath(const String& path) {
		String epath = path;

		for (size_t i = 0; i < epath.size(); i++) {
			if (epath[i] == OS_PATH_SEP)
				epath[i] = AX_PATH_SEP;
		}

		return epath;
	}

	String PathUtil::getRelativePath(const String& filename, const char* rootname) {
		if (rootname == nullptr)
			rootname = "axgame";
		String rootpath = "/";
		rootpath += rootname;
		rootpath += "/";

		String normalized = PathUtil::normalizePath(filename);
		size_t pos = normalized.find(rootpath);

		String result;
		if (pos == String::npos)
			return result;

		result = normalized.substr(pos + rootpath.size());

		return removeDirOne(result);
	}

	const char* PathUtil::skipDir(const char* pathname) {
		const char *last;

		last = pathname;
		while (*pathname) {
			if (PathUtil::isDirectoryLetter(*pathname))
				last = pathname+1;
			pathname++;
		}
		return last;
	}

	String PathUtil::removeFilename(const String& in) {
		String out = in;
		size_t len = out.length();

		for (len-=1; len; len--) {
			if (PathUtil::isDirectoryLetter(out[len])) {
				out.resize(len);
				break;
			}
		}

		if (len==0)
			out.clear();

		return out;
	}

	void PathUtil::toUnixPath(String& path) {
		size_t len = path.length();
		for (size_t i=0; i<len; i++) {
			if (path[i] == L'\\')
				path.at(i) = L'/';
		}
	}

	void PathUtil::convertSlash(String& path, char slash) {
		size_t len = path.length();
		for (size_t i=0; i<len; i++) {
			if (PathUtil::isDirectoryLetter(path[i]))
				path.at(i) = slash;
		}
	}

	bool PathUtil::getFileModifiedTime(const String& filename, longlong_t* t) {
		WIN32_FILE_ATTRIBUTE_DATA fileAttr;
		if (GetFileAttributesExW(u2w(filename).c_str(), GetFileExInfoStandard, &fileAttr)) {
			FILETIME time;
			time = fileAttr.ftLastWriteTime;

			if (t)
				(*t) = ((longlong_t)time.dwHighDateTime << 32) + time.dwLowDateTime;

			return true;
		}

		return false;
	}

	StringSeq PathUtil::findFiles(const String& dir, const String& filter, bool dironly) {
		uint_t flags = 0;
		if (dironly) {
			flags |= File::List_nofile;
		} else {
			flags |= File::List_nodirectory;
		}

		return listFileByExts(dir + '/', dir, filter, flags);
	}

	StringSeq PathUtil::listFileByExts(const String& base, const String& path, const String& exts, uint_t flags) {

		StringSeq strvec;
		FileInfoSeq fileinfos;

		fileinfos = getFileInfos(base, path, exts, flags);
		for (size_t i=0; i<fileinfos.size(); i++) {
			strvec.push_back(fileinfos[i].fullpath);
		}
		return strvec;
	}

	static bool CmpFileInfoNameLess(const FileInfo& left, const FileInfo& right) {
		if (left.isDir != right.isDir) {
			if (left.isDir)
				return true;
			else
				return false;
		}
		return left.filename < right.filename;
	}

	FileInfoSeq PathUtil::getFileInfos(const String& base, const String& path, const String& filters, int flags) {
		FileInfoSeq fileinfos;
		_wfinddata_t finddata;
		long handle;
		int num_files = 0;
		size_t baselen = base.length();

		Filter filter(filters);

		String pattern = path;
		if (!PathUtil::isDirectoryLetter(pattern[pattern.length() - 1])) {
			pattern += '/';
		}
		pattern += '*';

		handle = _wfindfirst(const_cast<wchar_t *>(u2w(pattern).c_str()), &finddata);
		if (handle == -1) {
			_findclose(handle);
			return fileinfos;
		}

		do {
			FileInfo info;
			if (finddata.attrib & _A_SUBDIR) {
				if (!wcscmp(finddata.name, L".") || !wcscmp(finddata.name, L"..")) {
					continue;
				}

				if (!(flags & File::List_needCVS)) {
					if (!wcscmp(finddata.name, L"CVS")) {
						continue;
					}
				}

				if (flags & File::List_nodirectory) {
					continue;
				}

				if (flags & File::List_filterDirectory) {
					if (!filter.In(w2u(finddata.name)))
						continue;
				}

				info.isDir = true;
			} else {
				if (flags & File::List_nofile) {
					continue;
				}

				if (!(flags & File::List_nofilterfile)) {
					if (!filter.In(w2u(finddata.name)))
						continue;
				}

				info.isDir = false;
			}

			info.fullpath = (path + "/" + w2u(finddata.name)).c_str() + baselen;
			info.filetype = File::Stdio;
			info.filesize = finddata.size;
			info.filetime = finddata.time_write;
			info.localtime = *_localtime64(&finddata.time_write);

			PathUtil::splitPath(info.fullpath, info.filepath, info.filename, info.fileext);
			info.filename = PathUtil::removeDir(info.fullpath);

			fileinfos.push_back(info);

			num_files++;

		} while (_wfindnext(handle, &finddata) != -1);

		_findclose(handle);

		if (flags & File::List_sort) {
			std::sort(fileinfos.begin(), fileinfos.end(), CmpFileInfoNameLess);
		}

		return fileinfos;
	}

	String PathUtil::cleanPath( const String& path )
	{
		if (path.empty())
			return path;
		String name = path;

		int used = 0, levels = 0;
		const int len = name.length();
		Sequence<char> out;
		out.resize(len);
		const char *p = name.c_str();
		for(int i = 0, last = -1, iwrite = 0; i < len; i++) {
			if(p[i] == ('/')) {
				while(i < len-1 && p[i+1] == ('/')) {
#if defined(_WIN32)
					if(!i)
						break;
#endif
					i++;
				}
				bool eaten = false;
				if(i < len - 1 && p[i+1] == ('.')) {
					int dotcount = 1;
					if(i < len - 2 && p[i+2] == ('.'))
						dotcount++;
					if(i == len - dotcount - 1) {
						if(dotcount == 1) {
							break;
						} else if(levels) {
							if(last == -1) {
								for(int i2 = iwrite-1; i2 >= 0; i2--) {
									if(out[i2] == ('/')) {
										last = i2;
										break;
									}
								}
							}
							used -= iwrite - last - 1;
							break;
						}
					} else if(p[i+dotcount+1] == ('/')) {
						if(dotcount == 2 && levels) {
							if(last == -1 || iwrite - last == 1) {
								for(int i2 = (last == -1) ? (iwrite-1) : (last-1); i2 >= 0; i2--) {
									if(out[i2] == ('/')) {
										eaten = true;
										last = i2;
										break;
									}
								}
							} else {
								eaten = true;
							}
							if(eaten) {
								levels--;
								used -= iwrite - last;
								iwrite = last;
								last = -1;
							}
						} else if (dotcount == 2 && i > 0 && p[i - 1] != ('.')) {
							eaten = true;
							used -= iwrite - std::max(0, last);
							iwrite = std::max(0, last);
							last = -1;
							++i;
						} else if(dotcount == 1) {
							eaten = true;
						}
						if(eaten)
							i += dotcount;
					} else {
						levels++;
					}
				} else if(last != -1 && iwrite - last == 1) {
#ifdef _WIN32
					eaten = (iwrite > 2);
#else
					eaten = true;
#endif
					last = -1;
				} else if(last != -1 && i == len-1) {
					eaten = true;
				} else {
					levels++;
				}
				if(!eaten)
					last = i - (i - iwrite);
				else
					continue;
			} else if(!i && p[i] == ('.')) {
				int dotcount = 1;
				if(len >= 1 && p[1] == ('.'))
					dotcount++;
				if(len >= dotcount && p[dotcount] == ('/')) {
					if(dotcount == 1) {
						i++;
						while(i+1 < len-1 && p[i+1] == ('/'))
							i++;
						continue;
					}
				}
			}
			out[iwrite++] = p[i];
			used++;
		}
		String ret;
		if(used == len)
			ret = name;
		else
			ret = String(&out[0], used);

		// Strip away last slash except for root directories
		if (ret[ret.length()-1] == '/' && !(ret.size() == 1 || (ret.size() == 3 && ret.at(1) == (':'))))
			ret.resize(ret.length() - 1);

		return ret;
	}

} // namespace Axon

