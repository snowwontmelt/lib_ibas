//
// Created by Павел on 17.05.2017.
//

#pragma once

//TODO adapt 'catch then dispose' strategy
//TODO handle subprocess signals

#include "../../fwd.h"

#define E4C_NOKEYWORDS
#include "e4c/e4c.h"
#undef E4C_NOKEYWORDS

#define $assert E4C_ASSERT

#define $try E4C_TRY

#define $catch E4C_CATCH

#define $finally E4C_FINALLY

#define $throw E4C_THROW

#define $rethrow(message_) E4C_RETHROW((message_) ? (message_) : e4c_get_exception()->message)

#define $_dispose(obj, failed) if (failed) Object.destroy(obj)
#define $with(resource) E4C_WITH(resource, $_dispose)

#define $_disposeAuto(obj, failed) Object.destroy(obj)
#define $withAuto(type, resource) \
  for (bool flag = true; flag; ) \
    for (type resource = NULL; flag; flag = false) \
      E4C_WITH(resource, $_disposeAuto)

#define $use E4C_USE

#define $declareException E4C_DECLARE_EXCEPTION

#define $defineException E4C_DEFINE_EXCEPTION

#include "exc-decl.h"
