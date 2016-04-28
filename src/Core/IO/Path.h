#pragma once

namespace Core {

class String;

    namespace IO {
        namespace Path {

String GetDirectoryName(const String &path);
String GetFileName(const String &path);
String GetFileExtension(const String &path);

        } // namespace Path
    } // namespace IO
} // namespace Core
