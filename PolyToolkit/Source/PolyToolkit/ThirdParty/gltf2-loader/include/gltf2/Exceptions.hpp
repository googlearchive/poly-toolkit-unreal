// MIT License
// 
// Copyright (c) 2017 The glTF2-loader Authors
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <string>
#include <exception>

namespace gltf2 {

class MisformattedException: public std::exception {
public:
    explicit MisformattedException(const char* key, const char* what) {
        _what = std::string("Misformated file: '") + key + "' " + what;
    }
    explicit MisformattedException(const std::string& key, const std::string& what) {
        _what = "Misformated file: '" + key + "' " + what;
    }
    virtual ~MisformattedException() throw() {}

    virtual const char* what() const throw() {
        return _what.c_str();
    }

protected:
    std::string _what;
};


class MisformattedExceptionNotNumber: public MisformattedException {
public:
    explicit MisformattedExceptionNotNumber(const char* key) : MisformattedException(key, "is not a number") {}
    explicit MisformattedExceptionNotNumber(const std::string& key) : MisformattedException(key, "is not a number") {}
    virtual ~MisformattedExceptionNotNumber() throw() {}
};

class MisformattedExceptionNotBoolean: public MisformattedException {
public:
    explicit MisformattedExceptionNotBoolean(const char* key) : MisformattedException(key, "is not a boolean") {}
    explicit MisformattedExceptionNotBoolean(const std::string& key) : MisformattedException(key, "is not a boolean") {}
    virtual ~MisformattedExceptionNotBoolean() throw() {}
};

class MisformattedExceptionNotString: public MisformattedException {
public:
    explicit MisformattedExceptionNotString(const char* key) : MisformattedException(key, "is not a string") {}
    explicit MisformattedExceptionNotString(const std::string& key) : MisformattedException(key, "is not a string") {}
    virtual ~MisformattedExceptionNotString() throw() {}
};

class MisformattedExceptionNotArray: public MisformattedException {
public:
    explicit MisformattedExceptionNotArray(const char* key) : MisformattedException(key, "is not an array") {}
    explicit MisformattedExceptionNotArray(const std::string& key) : MisformattedException(key, "is not an array") {}
    virtual ~MisformattedExceptionNotArray() throw() {}
};

class MisformattedExceptionNotGoodSizeArray: public MisformattedException {
public:
    explicit MisformattedExceptionNotGoodSizeArray(const char* key) : MisformattedException(key, "is not the good size") {}
    explicit MisformattedExceptionNotGoodSizeArray(const std::string& key) : MisformattedException(key, "is not the good size") {}
    virtual ~MisformattedExceptionNotGoodSizeArray() throw() {}
};

class MisformattedExceptionNotObject: public MisformattedException {
public:
    explicit MisformattedExceptionNotObject(const char* key) : MisformattedException(key, "is not an array") {}
    explicit MisformattedExceptionNotObject(const std::string& key) : MisformattedException(key, "is not an array") {}
    virtual ~MisformattedExceptionNotObject() throw() {}
};

class MisformattedExceptionIsRequired: public MisformattedException {
public:
    explicit MisformattedExceptionIsRequired(const char* key) : MisformattedException(key, "is required") {}
    explicit MisformattedExceptionIsRequired(const std::string& key) : MisformattedException(key, "is required") {}
    virtual ~MisformattedExceptionIsRequired() throw() {}
};

}
