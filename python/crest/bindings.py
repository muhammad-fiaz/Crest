"""
CFFI bindings to the Crest C library
"""

import os
import sys
from typing import Any, Optional
import cffi


class CrestLib:
    """Singleton wrapper for C library bindings"""

    _instance: Optional["CrestLib"] = None

    @classmethod
    def get_instance(cls) -> "CrestLib":
        """Get singleton instance"""
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def __init__(self) -> None:
        """Initialize CFFI and load library"""
        self.ffi = cffi.FFI()

        # Define C interface
        self.ffi.cdef("""
            typedef struct crest_app crest_app_t;
            typedef struct crest_request crest_request_t;
            typedef struct crest_response crest_response_t;
            
            typedef void (*crest_handler_fn)(crest_request_t*, crest_response_t*);
            
            // App functions
            crest_app_t* crest_create(void);
            void crest_destroy(crest_app_t* app);
            int crest_run(crest_app_t* app, const char* host, int port);
            void crest_enable_dashboard(crest_app_t* app, int enabled);
            
            // Route registration
            void crest_get(crest_app_t* app, const char* path, crest_handler_fn handler, const char* description);
            void crest_post(crest_app_t* app, const char* path, crest_handler_fn handler, const char* description);
            void crest_put(crest_app_t* app, const char* path, crest_handler_fn handler, const char* description);
            void crest_delete(crest_app_t* app, const char* path, crest_handler_fn handler, const char* description);
            void crest_patch(crest_app_t* app, const char* path, crest_handler_fn handler, const char* description);
            
            // Request functions
            const char* crest_request_method(crest_request_t* req);
            const char* crest_request_path(crest_request_t* req);
            const char* crest_request_body(crest_request_t* req);
            const char* crest_request_param(crest_request_t* req, const char* name);
            const char* crest_request_query(crest_request_t* req, const char* name);
            const char* crest_request_header(crest_request_t* req, const char* name);
            
            // Response functions
            void crest_response_status(crest_response_t* res, int status);
            void crest_response_header(crest_response_t* res, const char* name, const char* value);
            void crest_response_send(crest_response_t* res, const char* body);
            void crest_response_json(crest_response_t* res, const char* json);
        """)

        # Load the library
        self.lib = self._load_library()

    def _load_library(self) -> Any:
        """Load the Crest C library"""
        lib_name = self._get_library_name()
        lib_paths = self._get_library_paths()

        for path in lib_paths:
            lib_file = os.path.join(path, lib_name)
            if os.path.exists(lib_file):
                try:
                    return self.ffi.dlopen(lib_file)
                except OSError:
                    continue

        # Fallback: try to load from system
        try:
            return self.ffi.dlopen(lib_name)
        except OSError as e:
            raise RuntimeError(
                f"Could not load Crest library. "
                f"Please ensure the library is built and installed. "
                f"Searched paths: {lib_paths}"
            ) from e

    def _get_library_name(self) -> str:
        """Get platform-specific library name"""
        if sys.platform == "win32":
            return "crest.dll"
        elif sys.platform == "darwin":
            return "libcrest.dylib"
        else:
            return "libcrest.so"

    def _get_library_paths(self) -> list:
        """Get possible library paths"""
        paths = []

        # Build directory
        root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
        paths.append(os.path.join(root, "build"))
        paths.append(os.path.join(root, "build", "Release"))
        paths.append(os.path.join(root, "build", "Debug"))

        # System paths
        if sys.platform == "win32":
            paths.append(
                os.path.join(
                    os.environ.get("ProgramFiles", "C:\\Program Files"), "crest", "bin"
                )
            )
        else:
            paths.append("/usr/local/lib")
            paths.append("/usr/lib")

        return paths

    # Wrapper methods for C functions
    def create_app(self) -> Any:
        return self.lib.crest_create()

    def destroy_app(self, app: Any) -> None:
        self.lib.crest_destroy(app)

    def run_app(self, app: Any, host: str, port: int) -> int:
        return self.lib.crest_run(app, host.encode(), port)

    def enable_dashboard(self, app: Any, enabled: bool) -> None:
        self.lib.crest_enable_dashboard(app, 1 if enabled else 0)

    def add_route(
        self, app: Any, path: str, method: int, handler: Any, description: str
    ) -> None:
        c_handler = self.ffi.callback(
            "void(crest_request_t*, crest_response_t*)", handler
        )

        route_funcs = [
            self.lib.crest_get,
            self.lib.crest_post,
            self.lib.crest_put,
            self.lib.crest_delete,
            self.lib.crest_patch,
        ]

        if 0 <= method < len(route_funcs):
            route_funcs[method](app, path.encode(), c_handler, description.encode())

    def request_method(self, req: Any) -> str:
        result = self.lib.crest_request_method(req)
        return self.ffi.string(result).decode() if result else ""

    def request_path(self, req: Any) -> str:
        result = self.lib.crest_request_path(req)
        return self.ffi.string(result).decode() if result else ""

    def request_body(self, req: Any) -> str:
        result = self.lib.crest_request_body(req)
        return self.ffi.string(result).decode() if result else ""

    def request_param(self, req: Any, name: str) -> Optional[str]:
        result = self.lib.crest_request_param(req, name.encode())
        return self.ffi.string(result).decode() if result else None

    def request_query(self, req: Any, name: str) -> Optional[str]:
        result = self.lib.crest_request_query(req, name.encode())
        return self.ffi.string(result).decode() if result else None

    def request_header(self, req: Any, name: str) -> Optional[str]:
        result = self.lib.crest_request_header(req, name.encode())
        return self.ffi.string(result).decode() if result else None

    def response_status(self, res: Any, status: int) -> None:
        self.lib.crest_response_status(res, status)

    def response_header(self, res: Any, name: str, value: str) -> None:
        self.lib.crest_response_header(res, name.encode(), value.encode())

    def response_send(self, res: Any, body: str) -> None:
        self.lib.crest_response_send(res, body.encode())
