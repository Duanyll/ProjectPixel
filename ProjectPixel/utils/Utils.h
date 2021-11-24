#pragma once

#define NO_COPY_CLASS(className)          \
    className(const className&) = delete; \
    className& operator=(const className&) = delete;