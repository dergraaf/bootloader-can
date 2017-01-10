#!/usr/bin/env python3
#
# Copyright (c) 2010, 2015-2017 Fabian Greif.
# All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


class MessageDispatcher:

    def __init__(self, filterList = None):
        """Constructor"""
        self.filter = []
        if filterList:
            for f in filterList:
                self.addFilter(f)

    def addFilter(self, f):
        """Add a filter

        The filter-object must feature a check(message) method which returns
        True or False whether the callback should be called or not and a
        getCallback() method to retrieve this callback function.
        """
        self.filter.append(f)

    def removeFilter(self, f):
        """Remove this Filter"""
        self.filter.remove(f)

    def send(self, message):
        pass

    def _processMessage(self, message):
        """Check all filter for this message and call the callback
        functions for those how matches.
        """
        for f in self.filter:
            if f.check(message):
                self._executeCallback(f.getCallback(), message)

    def _executeCallback(self, callback, message):
        """Call a callback function."""
        callback(message)
