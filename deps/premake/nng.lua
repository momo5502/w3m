nng = {
	source = path.join(dependencies.basePath, "nng"),
}

function nng.import()
	links {
		"nng"
	}

	nng.includes()
end

function nng.includes()
	includedirs {
		path.join(nng.source, "include"),
	}

	defines {
		"NNG_PLATFORM_WINDOWS"
	}
end

function nng.project()
	project "nng"
		language "C"

		nng.includes()

		includedirs {
			path.join(nng.source, "src"),
		}

		files {
			path.join(nng.source, "src/**.c"),
			path.join(nng.source, "src/**.h"),
		}

		removefiles {
			path.join(nng.source, "src/platform/posix/**"),
			path.join(nng.source, "src/**/*test.c"),
			path.join(nng.source, "src/compat/**"),
			path.join(nng.source, "src/transport/tls/**"),
			path.join(nng.source, "src/transport/zerotier/**"),
			path.join(nng.source, "src/supplemental/tls/**"),
		}
		
		linkoptions {
			"-IGNORE:4006"
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, nng)
