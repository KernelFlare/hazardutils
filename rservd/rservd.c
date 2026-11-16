#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <grp.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

char *headOK = "HTTP/1.1 200 OK\r\n"
                "Server: rservd infdev\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "\r\n";
char *head404 = "HTTP/1.1 404 not found\r\n"
                "Server: rservd infdev\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "\r\n";
char *page404 = "<!DOCTYPE html><html>"
"    <head>"
"        <title>Rservd</title>"
"        <meta charset=\"UTF-8\">"
"        <style>"
"            .pane{position: relative;background: #00000000;left: 0;width: 96%;height: 64px;transition:transform 0.3s ease;margin: 1px 15px 1px 15px;color: #FF4444;align-content: left;outline: #440000;border-top: 1px #353535 solid;display: inline-block;}"
"            .pane h2{position: fixed;z-index: 3;font-size: 32px;margin-top: 16px;margin-bottom: 16px;padding: 0px;left: 96px;font-family: monospace;text-align: left;}"
"            .textpane{position: relative;background: #00000000;left: 0%;margin: 0 20px 0 20px;color: #bbbbbb;font-size: 22px;font-family: monospace;text-align: left;outline: #440000;border-bottom: 2px #555555 solid;}"
"            h2 {position: relative;z-index: 3;align-self: right;text-align:right;margin-top: 0px;padding-top: 30px;color: #dbdbdb;font-family: monospace;}"
"            @keyframes blink {"
"                0% {filter: brightness(0);}"
"                50% {filter: brightness(1);}"
"                100% {filter: brightness(0);}"
"            }"
"        </style>"
"    </head>"
"    <body style=\"background: linear-gradient(#0f0f0f, #272727);background-attachment: fixed;min-height: 100%;padding-top: 54px;margin: 16px;align-content: center;\">"
"        <header style=\"background: #0f0f0f;position: absolute;top: 0%;left: 0%;width: 100%;height: 52px;box-shadow: 0 2px 1px #ff0000;display: flex;\">"
"            <div style=\"padding: 6px;margin: 2px;position: absolute;text-align: center;top: 4px;height: 30px;left: 64px;font-family: monospace;margin: 2px 2px 2px 0px;background-color: black;color: #FF4444;\">"
"                <h2 style=\"position: relative;z-index: 3;font-size: 30px;margin-top: 0px;padding: 0px;font-family:monospace;color: #FF4444;\">/Error<span style=\"position: relative;z-index: 3;margin-top: 0px;font-family:monospace;animation: blink 1s infinite steps(1);\">_</span></h2>"
"            </div>"
"            <h2 style=\"padding: 3px;font-size: 32px;border: 2px #FF4444 solid;display: inline-block;border-radius: 2px;position: absolute;right: 64px;top: 4px;text-shadow: 0px 0px 5px #ff0000;color: #FF4444;background: linear-gradient(#d2d2d2, #f3f3f3);font-family:monospace;\">Rservd</h2>"
"        </header>"
"        <div class=\"textpane\">"
"            Uh-oh, thats not your fault"
"        </div>"
"        <div class=\"pane\">"
"            <h2>404 - not found</h2>"
"        </div>"
"        <div class=\"textpane\">Served using <a href=\"https://kernelflare.tailf177ce.ts.net/pub/srcbox/ports/braun.rservd.www\">Rservd</a></div>"
"    </body>"
"</html>\n";

char *head403 = "HTTP/1.1 403 forbidden\r\n"
                "Server: rservd infdev\r\n"
                "Connection: close\r\n"
                "Content-Type: text/html\r\n"
                "\r\n";
char *page403 = "<!DOCTYPE html><html>"
"    <head>"
"        <title>Rservd</title>"
"        <meta charset=\"UTF-8\">"
"        <style>"
"            .pane{position: relative;background: #00000000;left: 0;width: 96%;height: 64px;transition:transform 0.3s ease;margin: 1px 15px 1px 15px;color: #FF4444;align-content: left;outline: #440000;border-top: 1px #353535 solid;display: inline-block;}"
"            .pane h2{position: fixed;z-index: 3;font-size: 32px;margin-top: 16px;margin-bottom: 16px;padding: 0px;left: 96px;font-family: monospace;text-align: left;}"
"            .textpane{position: relative;background: #00000000;left: 0%;margin: 0 20px 0 20px;color: #bbbbbb;font-size: 22px;font-family: monospace;text-align: left;outline: #440000;border-bottom: 2px #555555 solid;}"
"            h2 {position: relative;z-index: 3;align-self: right;text-align:right;margin-top: 0px;padding-top: 30px;color: #dbdbdb;font-family: monospace;}"
"            @keyframes blink {"
"                0% {filter: brightness(0);}"
"                50% {filter: brightness(1);}"
"                100% {filter: brightness(0);}"
"            }"
"        </style>"
"    </head>"
"    <body style=\"background: linear-gradient(#0f0f0f, #272727);background-attachment: fixed;min-height: 100%;padding-top: 54px;margin: 16px;align-content: center;\">"
"        <header style=\"background: #0f0f0f;position: absolute;top: 0%;left: 0%;width: 100%;height: 52px;box-shadow: 0 2px 1px #ff0000;display: flex;\">"
"            <div style=\"padding: 6px;margin: 2px;position: absolute;text-align: center;top: 4px;height: 30px;left: 64px;font-family: monospace;margin: 2px 2px 2px 0px;background-color: black;color: #FF4444;\">"
"                <h2 style=\"position: relative;z-index: 3;font-size: 30px;margin-top: 0px;padding: 0px;font-family:monospace;color: #FF4444;\">/Error<span style=\"position: relative;z-index: 3;margin-top: 0px;font-family:monospace;animation: blink 1s infinite steps(1);\">_</span></h2>"
"            </div>"
"            <h2 style=\"padding: 3px;font-size: 32px;border: 2px #FF4444 solid;display: inline-block;border-radius: 2px;position: absolute;right: 64px;top: 4px;text-shadow: 0px 0px 5px #ff0000;color: #FF4444;background: linear-gradient(#d2d2d2, #f3f3f3);font-family:monospace;\">Rservd</h2>"
"        </header>"
"        <div class=\"textpane\">"
"            Hold on there pardner, you can use '..' on this website<br>You think that you're hacker or somethin'?"
"        </div>"
"        <div class=\"pane\">"
"            <h2>403 - forbidden</h2>"
"        </div>"
"        <div class=\"textpane\">Served using <a href=\"https://kernelflare.tailf177ce.ts.net/pub/srcbox/ports/braun.reserv.www\">Rservd</a></div>"
"    </body>"
"</html>\n";

char *version = "infdev";
char *poweredlink;
int usermode = 0;
struct utsname buffer;
const char *defaultpath = "/etc/rserv/www";
int active_threads = 0;
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static void err(char *line){
    fprintf(stderr ,"rservd_e: %s\n" ,line);
}
static void war(char *line, int va){
    if (va) fprintf(stderr ,"rservd_w: %s\n" ,line);
}
static void ver(char *line, int va){
    if (va) printf("rservd_v: %s\n" ,line);
}

int drop_privileges(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
        perror("user");
        return -1;
    }

    if (setgid(pw->pw_gid) != 0) {
        perror("setgid");
        return -1;
    }

    if (setgroups(1, &pw->pw_gid) != 0) {
        perror("setgroups");
        return -1;
    }

    if (setuid(pw->pw_uid) != 0) {
        perror("setuid:");
        return -1;
    }

    if (setuid(0) != -1) {
        perror("rserve_e: CRITICAL, got access to root user!\n");
        return -1;
    }

    printf("rservd: Dropped priveleges to '%s' user.\n", username);
    return 0;
}

void pathgen(void){
    if (access("/etc/rserv", F_OK) != 0) {
        if (mkdir("/etc/rserv", 0755) != 0 && errno != EEXIST) {
            perror("mkdir");
            return;
        }
    }
    if (access(defaultpath, F_OK) != 0) {
        if (mkdir(defaultpath, 0755) != 0 && errno != EEXIST) {
            perror("mkdir");
            return;
        }
    }
}

typedef struct {
    const char *ext;
    const char *mime;
} mime_type_t;
const mime_type_t mime_types[] = {   
    { "0" , "application/pkix-cert+pem" },
    { "4th" , "application/octet-stream" },
    { "7z",   "application/x-7z-compressed" },

    { "aac",  "audio/aac" },
    { "apk" , "application/vnd.android.package-archive" },
    { "atom" , "application/atom+xml" },
    { "atomcat" , "application/atomcat+xml" },
    { "avi",  "video/x-msvideo" },
    { "aw" , "application/applixware" },
    { "awk" , "application/x-awk" },

    { "bat",  "text/plain; charset=utf-8" },
    { "bin",  "application/octet-stream" },
    { "bz2",  "application/x-bzip2" },

    { "c",    "text/x-csrc" },
    { "cache" , "application/octet-stream" },
    { "cbdb" , "application/octet-stream" },
    { "ccxml" , "application/ccxml+xml" },
    { "cdmia" , "application/cdmi-capability" },
    { "cfg",  "text/plain; charset=utf-8" },
    { "cnf" , "text/plain; charset=utf-8" },
    { "cpp",  "text/x-c++src" },
    { "cs",  "text/x-csharp" },
    { "css",  "text/css; charset=utf-8" },
    { "csv" , "text/csv; charset=utf-8" },
    { "cw" , "application/gzip" },

    { "dat" , "application/octet-stream" },
    { "db" , "application/vnd.sqlite3" },
    { "dbk" , "application/docbook+xml" },
    { "dll" , "application/vnd.microsoft.portable-executable" },
    { "dsk" , "application/octet-stream" },
    { "dssc" , "application/dssc+der" },
    { "dts" , "text/x-devicetree-source" },

    { "ecma" , "application/ecmascript" },
    { "el" , "text/x-emacs-lisp" },
    { "eps" , "image/x-eps" },
    { "esdb" , "application/octet-stream" },
    { "exe",  "application/octet-stream" },
    { "ez" , "application/andrew-inset" },

    { "fig" , "image/x-xfig" },
    { "flac", "audio/flac" },
    { "fnt" , "text/x-font" },

    { "gif",  "image/gif" },
    { "gz",   "application/gzip" },

    { "h",    "text/x-chdr" },
    { "hex" , "text/x-hex" },
    { "hpp",  "text/x-c++hdr" },
    { "htm",  "text/html; charset=utf-8" },
    { "html", "text/html; charset=utf-8" },

    { "ico",  "image/x-icon" },
    { "img",  "application/octet-stream" },
    { "ini",  "text/plain; charset=utf-8" },
    { "iso",  "application/octet-stream" },

    { "jar",  "application/java-archive" },
    { "jpeg", "image/jpeg" },
    { "jpg",  "image/jpeg" },
    { "js",   "application/javascript; charset=utf-8" },
    { "json", "application/json; charset=utf-8" },

    { "kbd" , "text/plain; charset=utf-8" },

    { "log",  "text/plain; charset=utf-8" },
    { "lvl" , "application/gzip" },
    { "lz",   "application/x-lzip" },

    { "m4" , "application/x-m4" },
    { "m4a",  "audio/mp4" },
    { "md",   "text/markdown; charset=utf-8" },
    { "mk" , "text/x-makefile" },
    { "mkv",  "video/x-matroska" },
    { "mov",  "video/quicktime" },
    { "mp3",  "audio/mpeg" },
    { "mp4",  "video/mp4" },
    { "mps" , "application/octet-stream" },

    { "nix" , "application/octet-stream" },
    { "node", "application/octet-stream" },

    { "odt" , "application/vnd.oasis.opendocument.text" },
    { "oga",  "audio/ogg" },
    { "ogg",  "audio/ogg" },
    { "ogv",  "video/ogg" },
    { "old" , "application/x-trash" },
    { "otf" , "font/otf" },

    { "patch" , "text/x-patch" },
    { "pdb" , "chemical/x-pdb" },
    { "pdf" , "application/pdf" },
    { "pem" , "application/x-x509-ca-cert" },
    { "pfa" , "application/x-font-type1" },
    { "pkg" , "application/x-xar" },
    { "png", "image/png" },
    { "properties" , "text/plain; charset=utf-8" }, 
    { "pub" , "application/vnd.ms-publisher" },
    { "pvdb" , "application/octet-stream" },
    { "py" , "text/x-python" },

    { "qcow" , "application/x-qemu-disk" },

    { "rar",  "application/vnd.rar" },
    { "rc" , "text/plain; charset=utf-8" },
    { "readme" , "text/x-readme; charset=utf-8" },
    { "rom" , "application/octet-stream" },
    { "rs",   "text/rust" },
    { "rtf",  "application/rtf" },
    { "rules" , "application/octet-stream" },

    { "scm" , "text/x-scheme" },
    { "sh",   "application/x-sh" },
    { "so" , "application/x-sharedlib" },
    { "sql" , "application/sql" },
    { "stl",  "model/stl" },
    { "subr" , "application/octet-stream" },
    { "svg",  "image/svg+xml; charset=utf-8" },
    { "swf",  "application/x-shockwave-flash" },

    { "tar",  "application/x-tar" },
    { "tgz",  "application/gzip" },
    { "tmp",  "application/octet-stream" },
    { "toml", "application/toml" },
    { "ttf" , "font/ttf" },
    { "txt",  "text/plain; charset=utf-8" },
    { "txz",  "application/x-xz" },

    { "ucl" , "text/plain; charset=utf-8" },

    { "vbox-prev" , "application/xml; charset=utf-8" },
    { "vbox" , "application/xml; charset=utf-8" },
    { "vdi" , "application/x-vdi-disk" },
    { "vhd" , "application/x-vhd-disk" },

    { "wasm" , "application/wasm" },
    { "wav",  "audio/wav" },
    { "webm", "video/webm" },
    { "webp", "image/webp" },

    { "woff", "font/woff" },
    { "woff2",  "font/woff2" },

    { "xdssc" , "application/dssc+xml; charset=utf-8" },
    { "xml",  "application/xml; charset=utf-8" },
    { "xz",   "application/x-xz" },

    { "yaml", "text/yaml; charset=utf-8" },
    { "yml",  "text/yaml; charset=utf-8" },

    { "zip",  "application/zip" },
    { NULL,   NULL }
};
int compare_mime(const void *a, const void *b) {
    const mime_type_t *m1 = (const mime_type_t *)a;
    const mime_type_t *m2 = (const mime_type_t *)b;
    return strcasecmp(m1->ext, m2->ext);
}

char *content_type(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) {
        return "application/octet-stream";
    }
    mime_type_t key;
    key.ext = dot + 1;
    key.mime = NULL;
    size_t num_elements = (sizeof(mime_types) / sizeof(mime_types[0])) - 1;
    mime_type_t *found = bsearch(&key, mime_types, num_elements, 
                                 sizeof(mime_type_t), compare_mime);
    if (found != NULL) {
        return (char *)found->mime;
    }
    return "application/octet-stream";
}

void *handle_client(void *arg) {
    int client = (int)(intptr_t)arg;
    char request[4096];
    ssize_t n = read(client, request, sizeof(request) - 1);
    if (n <= 0) {
        close(client);
        goto cleanup;
    }
    request[n] = '\0';
    char root_dir[PATH_MAX];
    if (usermode) {
        realpath("www", root_dir);
    } else {
        realpath(defaultpath, root_dir);
    }
    char method[8], path[256];
    if (sscanf(request, "%7s %255s", method, path) != 2) {
        close(client);
        goto cleanup;
    }
    printf("%s %s\n", method, path);
    int ishead = 0;
    if (strcmp(method,"HEAD") == 0) ishead = 1;
    if (strcmp(path,"/about:server") == 0){
        char about[4096];
        snprintf(about, sizeof(about), 
            "<!DOCTYPE html><html><head><title>Rservd - Info</title><meta charset=\"UTF-8\"><style>header{background: #0f0f0f;position: absolute;top: 0%%;left: 0%%;width: 100%%;height: 52px;box-shadow: 0 2px 1px #ff0000;display: flex;}body{background: linear-gradient(#0f0f0f, #272727);background-attachment: fixed;min-height: 100%%;padding-top: 54px;margin: 16px;align-content: center;}.label{padding: 3px;font-size: 32px;border: 2px #FF4444 solid;display: inline-block;border-radius: 2px;position: absolute;right: 64px;top: 4px;text-shadow: 0px 0px 5px #ff0000;color: #FF4444;background: linear-gradient(#d2d2d2, #f3f3f3);font-family:system-ui, -apple-system;}.label:active{background: linear-gradient(#f3a3a3, #d29292);text-shadow: 0px 0px 5px #ff7f7f;border: 3px #ff1f1f solid;}.textpane{position: relative;background: #00000000;left: 0%%;margin: 0 20px 0 20px;color: #bbbbbb;font-size: 22px;font-family: monospace;text-align: left;outline: #440000;border-bottom: 2px #555555 solid;}.pane{position: relative;background: #00000000;left: 0;width: 96%%;min-height: 64px;transition:transform 0.3s ease;margin: 1px 15px 1px 15px;color: #FF4444;align-content: left;outline: #440000;border-top: 1px #353535 solid;display: inline-block;}.pane h2{position: fixed;z-index: 3;font-size: 32px;margin-top: 16px;margin-bottom: 16px;padding: 0px;left: 96px;font-family: monospace;text-align: left;}.pane img{height: 64px;right: -480px;position: relative;}.pane:hover{background: #52525222;}.type{padding: 6px;margin: 2px;position: absolute;text-align: center;top: 4px;height: 30px;left: 64px;font-family: 'admin', system-ui, -apple-system;margin: 2px 2px 2px 0px;background-color: black;color: #FF4444;}.type h2{position: relative;z-index: 3;font-size: 30px;margin-top: 0px;padding: 0px;font-family:monospace;color: #FF4444;}.blink{position: relative;z-index: 3;margin-top: 0px;font-family:monospace;animation: blink 1s infinite steps(1);}a{color: #ff1f1f;text-decoration: none;}h2{position: relative;z-index: 3;align-self: right;text-align:right;margin-top: 0px;padding-top: 30px;color: #dbdbdb;font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;}@keyframes blink {0%% {filter: brightness(0);}50%% {filter: brightness(1);}100%% {filter: brightness(0);}}</style></head><body><header><div class=\"type\"><h2>/about:server<span id=\"blink\" class=\"blink\">_</span></h2></div><h2 class=\"label\" id=\"label\" onclick=\"window.open('http://kernelflare.c-om.net','width=748,height=420'); document.getElementById('blink').innerText = '█';\">Rservd</h2></header><div class=\"textpane\">Running system stats<div class=\"pane\"><h2>Kernel - %s, btw.</h2><img src=\"%s\" alt=\"\"></div><div class=\"pane\"><h2>Architecture - %s</h2></div><div class=\"textpane\">Served by <a href=\"http://kernelflare.c-om.net/pub/srcbox/ports/braun.reserv.www\">Rservd</a><br>Rservd contact & bugtracker: <a href=\"mailto:kernelflare@gmail.com\">kernelflare@gmail.com</a></div></body></html>\n""",
        buffer.sysname, poweredlink, buffer.machine);
        write(client, headOK, strlen(headOK));
        if (ishead == 0)write(client, about, strlen(about));
        close(client);
        goto cleanup;
    }
    else if (strcmp(path,"/about:blank") == 0){
        char about[128];
        snprintf(about, sizeof(about),
            "<html><head></head><body></body></html>\n""");
        write(client, headOK, strlen(headOK));
        if (ishead == 0)write(client, about, strlen(about));
        close(client);
        goto cleanup;
    }
    else if (strcmp(path,"/about:test") == 0){
        char about[196];
        snprintf(about, sizeof(about), 
            "<html><head></head><body><h1>Rservd test, btw</h1></body></html>\n""");
        write(client, headOK, strlen(headOK));
        if (ishead == 0)write(client, about, strlen(about));
        close(client);
        goto cleanup;
    }
    else {
        if(strstr(path, ".." ) != NULL || strstr(path, "%2e%2e" )){
            write(client, head403, strlen(head403));
            if (ishead == 0)write(client, page403, strlen(page403));
            close(client);
            goto cleanup;
        }
        char filename[512];
        char *target = path;
        size_t len = strlen(path);
        if (len > 0 && path[len - 1] == '/') {
            const char *defaults[] = {"index.html", "index.htm", NULL};
            static char new_target[1024];
            for (int i = 0; defaults[i] != NULL; i++) {
                char temp_path[1024];
                if (usermode == 0) snprintf(temp_path, sizeof(temp_path), "%s%s%s", defaultpath, path, defaults[i]);
                else snprintf(temp_path, sizeof(temp_path), "www%s%s", path, defaults[i]);
                if (access(temp_path, F_OK) == 0) {
                    snprintf(new_target, sizeof(new_target), "%s%s", path, defaults[i]);
                    target = new_target;
                    break;
                }
            }
        }
        if(usermode == 0)snprintf(filename, sizeof(filename),"%s%s",defaultpath, target);
        else snprintf(filename, sizeof(filename), "www%s", target);
        char resolved_path[PATH_MAX];
        struct stat info;
        int file = -1;
        if (realpath(filename, resolved_path) == NULL || (file = open(resolved_path, O_RDONLY | O_NOFOLLOW)) < 0){
            write(client, head404, strlen(head404));
            if (ishead == 0)write(client, page404, strlen(page404));
            close(client);
            goto cleanup;
        }
        if (strncmp(resolved_path, root_dir, strlen(root_dir)) != 0) {
            close(file);
            write(client, head403, strlen(head403));
            if (ishead == 0)write(client, page403, strlen(page403));
            close(client);
            goto cleanup;
        }
        fstat(file, &info);
        if (S_ISDIR(info.st_mode)) {
            close(file);
            char *pageheader = "<!DOCTYPE html><html>"
"    <head>"
"        <title>Rservd</title>"
"        <meta charset=\"UTF-8\">"
"        <style>"
"            .pane{position: relative;background: #00000000;left: 0;width: 96%;height: 64px;transition:transform 0.3s ease;margin: 1px 15px 1px 15px;color: #FF4444;align-content: left;outline: #440000;border-top: 1px #353535 solid;display: inline-block;}"
"            .pane h2{position: fixed;z-index: 3;font-size: 32px;margin-top: 16px;margin-bottom: 16px;padding: 0px;left: 96px;font-family: monospace;text-align: left;}"
"            .textpane{position: relative;background: #00000000;left: 0%;margin: 0 20px 0 20px;color: #bbbbbb;font-size: 22px;font-family: monospace;text-align: left;outline: #440000;border-bottom: 2px #555555 solid;}"
"            h2 {position: relative;z-index: 3;align-self: right;text-align:right;margin-top: 0px;padding-top: 30px;color: #dbdbdb;font-family: monospace;}"
"            @keyframes blink {"
"                0% {filter: brightness(0);}"
"                50% {filter: brightness(1);}"
"                100% {filter: brightness(0);}"
"            }"
"        </style>"
"    </head>"
"    <body style=\"background: linear-gradient(#0f0f0f, #272727);background-attachment: fixed;min-height: 100%;padding-top: 54px;margin: 16px;align-content: center;\">"
"        <header style=\"background: #0f0f0f;position: absolute;top: 0%;left: 0%;width: 100%;height: 52px;box-shadow: 0 2px 1px #ff0000;display: flex;\">"
"            <div style=\"padding: 6px;margin: 2px;position: absolute;text-align: center;top: 4px;height: 30px;left: 64px;font-family: monospace;margin: 2px 2px 2px 0px;background-color: black;color: #FF4444;\">"
"                <h2 style=\"position: relative;z-index: 3;font-size: 30px;margin-top: 0px;padding: 0px;font-family:monospace;color: #FF4444;\">/AutoIndex<span style=\"position: relative;z-index: 3;margin-top: 0px;font-family:monospace;animation: blink 1s infinite steps(1);\">_</span></h2>"
"            </div>"
"            <h2 style=\"padding: 3px;font-size: 32px;border: 2px #FF4444 solid;display: inline-block;border-radius: 2px;position: absolute;right: 64px;top: 4px;text-shadow: 0px 0px 5px #ff0000;color: #FF4444;background: linear-gradient(#d2d2d2, #f3f3f3);font-family:monospace;\">Rservd</h2>"
"        </header>";
            write(client, headOK, strlen(headOK));
            if (ishead == 0){write(client, pageheader, strlen(pageheader));
                DIR *dir = opendir(filename);
                if (dir) {
                    struct dirent *entry;
                    while ((entry = readdir(dir)) != NULL) {
                        int is_dot = (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0);
                        char link[1024];
                        const char *slash = (path[strlen(path) - 1] == '/') ? "" : "/";
                            snprintf(link, sizeof(link), 
                                "<a href=\"%s%s%s\" %s style=\"text-decoration: none; color: inherit; display: block;\">"
                                        "    <div class=\"pane\">"
                                        "        <h2>%s</h2>"
                                        "    </div>"
                                        "</a>", 
                        path, slash, entry->d_name, is_dot ? "rel=\"nofollow\"" : "", entry->d_name);
                        write(client, link, strlen(link));
                    }
                    closedir(dir);
                }
                char *footer = 
                "        <div class=\"textpane\">Served using <a href=\"https://kernelflare.tailf177ce.ts.net/pub/srcbox/ports/braun.reserv.www\">Rservd</a></div>"
                "    </body>"
                "</html>\n";
            write(client, footer, strlen(footer));}
            close(client);
            goto cleanup;
        }
        if(!S_ISREG(info.st_mode)){
            close(file);
            write(client, head404, strlen(head404));
            if (ishead == 0)write(client, page404, strlen(page404));
            close(client);
            goto cleanup;
        }
    char superheader[512];
    snprintf(superheader, sizeof(superheader),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Server: rservd infdev\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type(filename), (long)info.st_size);
    write(client, superheader, strlen(superheader));
    if(ishead == 0){
        #if defined(__FreeBSD__)
        off_t sent_bytes = 0;
        off_t offset = 0;
        int ret;
        while (offset < info.st_size) {
            ret = sendfile(file, client, offset, info.st_size - offset, NULL, &sent_bytes, 0);
            if (ret == -1) {
                break; 
            }
            if (sent_bytes == 0) {
                break;
            }
            offset += sent_bytes;
        }
        #else
        char buffer[8192];
        ssize_t count;
        while((count = read(file, buffer, sizeof(buffer))) > 0){
            if (write(client, buffer, count) < 0) {
                break;
            }
        }
        #endif
    }
    close(file);
    close(client);
    goto cleanup;
    }
    cleanup:
    pthread_mutex_lock(&thread_mutex);
    active_threads--;
    pthread_mutex_unlock(&thread_mutex);
    return NULL;
}

int main(int argc, char *argv[]){
    if (uname(&buffer) != 0) {
        perror("bname: uname failed");
        return 1;
    }
    if (strcmp(buffer.sysname, "Linux") == 0) poweredlink = "https://upload.wikimedia.org/wikipedia/commons/d/d5/Webpage_icon-powered_by_linux.svg";
    else if (strcmp(buffer.sysname, "FreeBSD") == 0) poweredlink = "https://www.freebsd.org/gifs/powerlogo.gif";
    else poweredlink = "unknown";
    int opts;
    int verbose = 0;
    int port = 0;
    int backlog = 0;
    int max_threads = 10;
    while((opts = getopt(argc, argv, "p:vb:ut:")) != -1){
        switch (opts) {
            case 'v':
                verbose = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'b':
                backlog = atoi(optarg);
                break;
            case 'u':
                usermode = 1;
                break;
            case 't':
                max_threads = atoi(optarg);
                if (max_threads <= 0) max_threads = 1;
                break;
            default:
                err(
                    "OPTIONS:\n"
                    "   -p  port    Port number to bint the server to.\n"
                    "   -v          Enable verbose output mode.\n"
                    "   -b  backlog Maximum lenght to which the queue of pending.\n"
                    "   -u          enables user mode hosting in ./www dir\n"
                    "               if not enabled, uses /etc/rserv/www.\n"
                    "   -t  threads Changes max threads count.");
                exit(1);
        }
    }

    int server = socket(AF_INET, SOCK_STREAM, 0);

    

    if (server < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (usermode == 1) {
        ver("option -u was set, serving ./www directory", verbose);
         if(access(defaultpath, F_OK)!= 0) err("Coudn't find ./www dir");
    }

    if (port == 0) {
        address.sin_port = htons(8080);
        war("option -p was unset, using default port - :8080", verbose);
    }


    else {
        address.sin_port = htons(port);
        printf("rservd: using port - :%d (%d)\n", port, address.sin_port);
    }

    if (backlog == 0){
        backlog = 10;
        war("option -b was unset, using default length - 10", verbose);
    }

    else printf("rservd: using backlog length - %d\n", backlog);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }

    if (usermode == 0){
        pathgen();
        if (getuid() == 0) {
            if (drop_privileges("www") != 0) {
                err("Failed to drop privileges. Running as root.");
            } else printf("rservd: privileges dropped to 'www'\n");
        } else printf("rservd: running as non-root user, skipping privilege drop.\n");
    } else {
        printf("rservd: user mode enabled, serving ./www directory\n");
        if (access("www", F_OK) != 0) war("warning: ./www directory not found. All requests (excl: /about:server, /about:test, /about:blank and all pages with '..' pattern) will return 404.", verbose);
    }

    if (listen(server,10) < 0){
        perror("listen");
        exit(1);
    }

    while(1){
    int client = accept(server, NULL, NULL);
    if (client < 0){
        perror("accept");
        continue;
    }

    pthread_mutex_lock(&thread_mutex);

    if (active_threads >= max_threads) {
        pthread_mutex_unlock(&thread_mutex);
        
        char *busy_header = "HTTP/1.1 503 Service Unavailable\r\n"
                            "Server: rservd infdev\r\n"
                            "Connection: close\r\n\r\n"
                            "Server is too busy. Try again later.";
        write(client, busy_header, strlen(busy_header));
        close(client);
        
            war("max threads reached, dropping client",1);
            continue;
        }

        active_threads++;
        pthread_mutex_unlock(&thread_mutex);

        if (verbose) printf("rservd: client fd %d (threads: %d/%d)\n", client, active_threads, max_threads);

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)client) != 0) {
            perror("pthread_create");
            close(client);
        
            pthread_mutex_lock(&thread_mutex);
            active_threads--;
            pthread_mutex_unlock(&thread_mutex);
        } else {
            pthread_detach(thread);
        }
    }
    if (verbose == 1) printf("rservd_v: socket fd %d\n", server);
    return 0;
}
