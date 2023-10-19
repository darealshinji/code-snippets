/***

$ cat ~/.config/user-dirs.dirs
# This file is written by xdg-user-dirs-update
# If you want to change or add directories, just edit the line you're
# interested in. All local changes will be retained on the next run.
# Format is XDG_xxx_DIR="$HOME/yyy", where yyy is a shell-escaped
# homedir-relative path, or XDG_xxx_DIR="/yyy", where /yyy is an
# absolute path. No other format is supported.
# 
XDG_DESKTOP_DIR="$HOME/Schreibtisch"
XDG_DOWNLOAD_DIR="$HOME/Downloads"
XDG_TEMPLATES_DIR="$HOME/"
XDG_PUBLICSHARE_DIR="$HOME/"
XDG_DOCUMENTS_DIR="$HOME/Dokumente"
XDG_MUSIC_DIR="$HOME/Musik"
XDG_PICTURES_DIR="$HOME/Bilder"
XDG_VIDEOS_DIR="$HOME/Videos"

***/

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <stdlib.h>
#include <strings.h>


class xdg
{
private:

    struct dir {
        std::string var;
        std::string path;
        std::string basename;
    };

    std::string m_home, m_config, m_locale;
    std::vector<struct dir> m_paths;

    bool get_home()
    {
        if (!m_home.empty()) {
            return true;
        }

        char *p = getenv("HOME");
        if (!p) return false;

        m_home = p;

        return m_home.empty() ? false : true;
    }

public:

    xdg() {}

    virtual ~xdg() {}


    bool load(const std::string &file)
    {
        clear();
        m_config = file;

        /* load config file */
        std::ifstream ifs(m_config.c_str());

        if (!ifs.is_open()) {
            return false;
        }

        const std::regex reg("XDG_([A-Za-z]*)_DIR=\"(.*?)\"");
        std::string line;

        while (std::getline(ifs, line)) {
            struct dir path;
            std::smatch m;

            /* comment */
            if (line.starts_with('#')) {
                continue;
            }

            if (!std::regex_match(line, m, reg) || m.size() != 3) {
                continue;
            }

            path.var = m[1];
            path.path = m[2];

            if (path.var.empty() || path.path.empty()) {
                continue;
            }

            /* strip trailing slashes */
            if (path.path.back() == '/') {
                while (path.path.back() == '/') {
                    path.path.pop_back();
                }

                /* path was only slashes */
                if (path.path.empty()) {
                    path.path = "/";
                }
            }

            /* path equals $HOME */
            if (path.path == "$HOME") {
                continue;
            }

            if (path.path.starts_with("$HOME")) {
                /* replace $HOME variable */
                if (!get_home()) {
                    return false;
                }
                path.path.replace(0, 5, m_home);
            } else if (!path.path.starts_with('/')) {
                /* path must be absolute */
                continue;
            }

            /* path equals $HOME */
            if (get_home() && path.path == m_home) {
                continue;
            }

            /* get the basename */
            auto pos = path.path.rfind('/');

            if (pos == std::string::npos) {
                continue;
            }

            path.basename = path.path.substr(pos + 1);

            if (path.basename.empty()) {
                continue;
            }

            /* replace existing entry */
            bool found = false;

            for (auto &e : m_paths) {
                if (e.var == path.var) {
                    e.path = path.path;
                    e.basename = path.basename;
                    found = true;
                }
            }

            if (!found) {
                m_paths.push_back(path);
            }
        }

        /* sort by basename */
        auto comp = [] (const struct dir &a, const struct dir &b) -> bool {
            return strcasecmp(a.basename.c_str(), b.basename.c_str()) < 0;
        };

        std::sort(m_paths.begin(), m_paths.end(), comp);

        return true;
    }


    bool load()
    {
        if (!get_home()) {
            return false;
        }

        /* load default config file */
        return load(m_home + "/.config/user-dirs.dirs");
    }


    std::string locale()
    {
        if (!m_locale.empty()) {
            return m_locale;
        }

        if (!m_config.ends_with(".dirs")) {
            return {};
        }

        std::string file = m_config;
        file.replace(file.size()-4, 4, "locale");

        std::ifstream ifs(file.c_str());

        if (!ifs.is_open()) {
            return {};
        }

        std::getline(ifs, m_locale);

        return m_locale;
    }


    void clear()
    {
        m_paths.clear();
        m_config.clear();
        m_locale.clear();
    }


    size_t size() const {
        return m_paths.size();
    }


    std::string var(size_t pos) const {
        return m_paths.at(pos).var;
    }


    std::string path(size_t pos) const {
        return m_paths.at(pos).path;
    }


    std::string path(const std::string &var) const
    {
        for (const auto &path : m_paths) {
            if (path.var == var) {
                return path.path;
            }
        }

        return {};
    }


    std::string basename(size_t pos) const {
        return m_paths.at(pos).basename;
    }


    std::string basename(const std::string &var) const
    {
        for (const auto &path : m_paths) {
            if (path.var == var) {
                return path.basename;
            }
        }

        return {};
    }


    std::string home()
    {
        get_home();
        return m_home;
    }
};


int main()
{
    xdg dirs;

    if (!dirs.load()) {
        return 1;
    }

    std::cout << "desktop == " << dirs.path("DESKTOP") << std::endl;
    std::cout << "locale: " << dirs.locale() << std::endl;

    for (size_t i = 0; i < dirs.size(); i++) {
        std::cout << dirs.var(i) << ": "
            << dirs.basename(i) << " -> "
            << dirs.path(i) << std::endl;
    }

    return 0;
}
