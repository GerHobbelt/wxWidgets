#pragma once

#include "framework.h"

#pragma warning(disable : 4250 4251 4267 4275 4244 5030)
#include <qapplication.h>
#include <qmainwindow.h>
#include <qmdiarea.h>
#include <qmdisubwindow.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <set>
#include <map>
#include <filesystem>
#include <future>

#include <boost/dll.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include "logger.h"
#include "screen_coord_converter.h"
#include "pcb_loader.h"
#include "database.h"
