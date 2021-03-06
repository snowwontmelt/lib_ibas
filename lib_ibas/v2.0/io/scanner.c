//
// Created by Павел on 21.12.2016.
//

#include "scanner.h"
#include "../base/base.h"

//TODO do not always switch params

$defineException(EmptyTokenException, "Scanner: unable to produce next token!", FormatException);

static Scanner_t create(FILE *stream, String_t str, Pointer_t iter) {
  Scanner_t self = Ibas.alloc(sizeof(Scanner_s), NULL);

  self->class = Scanner.class;
  self->multiline = true;
  self->delimiters = " \t\n";
  self->last = 0;
  self->destroySource = false;

  self->stream = stream;

  if (str && !iter) iter = String.begin(str);
  self->iter = iter;
  self->str = str;

  return self;
}

static Scanner_t fromStream(FILE *stream) {
  return create(stream, NULL, NULL);
}

static Scanner_t fromString(String_t str, Pointer_t iter) {
  return create(NULL, str, iter);
}

static void destroy(Scanner_t self) {
  if (!self) return;

  if (self->destroySource) {
    if (self->str) String.destroy(self->str);
    if (self->stream) File_w.destroy(&self->stream);
  }

  free(self);
}

static String_t toString(Scanner_t self) {
  return String.format("[[Scanner stream=%p str=%p iter=%p]]", self->stream, self->str, self->iter);
}

static int compare(Scanner_t obj1, Scanner_t obj2) {
  $throw(NotImplementedException, "Scanner.compare() is not implemented!");
  return 0;
}

static void serialize(Scanner_t self, Writer_t writer) {
  $throw(NotImplementedException, "Scanner.serialize() is not implemented!");
}

static Scanner_t deserialize(Scanner_t self, Scanner_t scanner) {
  $throw(NotImplementedException, "Scanner.deserialize() is not implemented!");
  return NULL;
}

static unsigned read(Scanner_t self, String_t buffer) {
  char ch = 0;
  bool eof = false;
  unsigned count = 0;

  Scanner.skipSpace(self);

  while (true) {
    //@formatter:off
    $try {
      ch = Scanner.nextChar(self);
    } $catch(EOFException) {
      eof = true;
    }
    //@formatter:on

    if (eof) break;

    if (strchr(self->delimiters, ch) || (ch == '\n' && !self->multiline)) {
      self->last = ch;
      break;
    }

    count++;
    if (buffer) String.add(buffer, ch);
  }

  return count;
}

static String_t nextToken(Scanner_t self) {
  String_t buffer = String.create(0);
  read(self, buffer);
  return buffer;
}

static void nextFormat(Scanner_t self, CString_t format, Pointer_t dest) {
  //@formatter:off
  $withAuto(String_t, token) {
    token = Scanner.nextToken(self);
  } $use {
    if (!token->size) {
      $throw(EmptyTokenException, NULL);
    }

    String_t _format = CString_w.toString(format);
    String.appendCStr(_format, "%n");

    int count;
    int ret = sscanf(String.CStr(token), String.CStr(_format), dest, &count);

    if (ret < 0) $throw(RuntimeException, "Scanner: unknown error!");
    if (ret == 0 || count < token->size) $throw(FormatException, NULL);
  };
  //@formatter:on
}

static char nextChar(Scanner_t self) {
  char ch = 0;

  if (self->last) {
    ch = self->last;
  } else if (self->stream && !feof(self->stream)) {
    ch = (char) fgetc(self->stream); //TODO check
  } else if (self->str && self->iter != String.end(self->str)) {
    ch = String.iterGet(self->str, self->iter);
    self->iter = String.iterNext(self->str, self->iter);
  } else {
    $throw(EOFException, NULL);
  }

  self->last = 0;
  return ch;
}

static int nextInt(Scanner_t self) {
  int ret = 0;
  Scanner.nextFormat(self, "%d", &ret);
  return ret;
}

static double nextDouble(Scanner_t self) {
  double ret = 0;
  Scanner.nextFormat(self, "%lf", &ret);
  return ret;
}

static String_t nextLine(Scanner_t self) {
  CString_t delims = self->delimiters;
  self->delimiters = "\n";
  bool multi = self->multiline;
  self->multiline = false;

  String_t ret = Scanner.nextToken(self);
  self->last = 0;

  self->delimiters = delims;
  self->multiline = multi;
  return ret;
}

static String_t nextText(Scanner_t self) {
  //TODO implement
  return NULL;
}

static unsigned skip(Scanner_t self) {
  return read(self, NULL);
}

static unsigned skipSpace(Scanner_t self) {
  unsigned count = 0;
  char ch = 0;
  bool eof = false;

  while (true) {
    //@formatter:off
    $try {
      ch = Scanner.nextChar(self);
    } $catch(EOFException) {
      eof = true;
    }
    //@formatter:on

    if (eof) break;

    if (!strchr(self->delimiters, ch) || (ch == '\n' && !self->multiline)) {
      self->last = ch;
      break;
    }

    count++;
  }

  return count;
}

static unsigned skipLine(Scanner_t self) {
  unsigned count = 0;
  while (Scanner.nextChar(self) != '\n') count++;
  return count;
}

//skips the current line returns true if there was at least one token
static bool endLine(Scanner_t self) {
  bool ret = false;
  bool multi = self->multiline;
  self->multiline = false;

  if (Scanner.skip(self) > 0) ret = true;
  Scanner.skipLine(self);

  self->multiline = multi;
  return ret;
}

static void match(Scanner_t self, CString_t format) {
  while (*format && strchr(self->delimiters, *format)) format++;
  Scanner.skipSpace(self);

  while (*format && !strchr(self->delimiters, *format)) {
    char ch = Scanner.nextChar(self);
    if (ch != *format) {
      self->last = ch;
      $throw(FormatException, NULL);
    }
    format++;
  }
}

static bool matches(Scanner_t self, CString_t format) {
  bool good = false;
  //@formatter:off
    $try {
      Scanner.match(self, format);
      good = true;
    } $catch(EOFException) {
    } $catch(FormatException) {
    };
    //@formatter:on
  return good;
}

$defineNamespace(Scanner) {
    (Class_t) &Scanner.destroy,
    fromStream,
    fromString,
    destroy,
    toString,
    compare,
    serialize,
    deserialize,
    nextToken,
    nextFormat,
    nextChar,
    nextInt,
    nextDouble,
    nextLine,
    nextText,
    skip,
    skipSpace,
    skipLine,
    endLine,
    match,
    matches
};
