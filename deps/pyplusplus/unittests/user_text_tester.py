# Copyright 2004 Roman Yakovenko.
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

import os
import sys
import unittest
import fundamental_tester_base

class tester_t(fundamental_tester_base.fundamental_tester_base_t):
    EXTENSION_NAME = 'user_text'
    
    def __init__( self, *args ):
        fundamental_tester_base.fundamental_tester_base_t.__init__( 
            self
            , tester_t.EXTENSION_NAME
            , *args )
                                                                    
    def customize(self, mb ):
        data = mb.class_( 'data' )
        class_code = "/*class code*/"
        data.add_registration_code( class_code, False )
        wrapper_code = "/*wrapper code*/" 
        data.add_wrapper_code( wrapper_code )
        mb.build_code_creator( self.EXTENSION_NAME )
        code = mb.code_creator.create()
        self.failUnless( ( class_code in code ) and ( wrapper_code in code ) )
        
    def run_tests(self, module):        
        pass
    
def create_suite():
    suite = unittest.TestSuite()    
    suite.addTest( unittest.makeSuite(tester_t))
    return suite

def run_suite():
    unittest.TextTestRunner(verbosity=2).run( create_suite() )

if __name__ == "__main__":
    run_suite()