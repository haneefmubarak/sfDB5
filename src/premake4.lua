solution "sfDB5"
	configurations { "Debug", "Release" }

	configuration "Debug"
		defines { "_GNU_SOURCE" }
		includedirs { "../deps" }
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-pthread" }
		buildoptions {	"-Wno-pointer-sign",
				"-Wno-unused-function",
				"-Wno-unused-but-set-variable",
				"-Wno-unused-parameter",
				"-Wno-incompatible-pointer-types-discards-qualifiers",
				"-Wno-discarded-qualifiers",
				"-Wno-missing-field-initializers",
				"-Wno-old-style-declaration",
				"-Wno-unknown-warning-option",
		}
		linkoptions { "-pthread" }

	configuration "Release"
		defines { "_GNU_SOURCE" }
		includedirs { "../deps" }
		buildoptions { "-pthread", "-march=native", "-O2" }
		buildoptions {	"-Wno-pointer-sign",
				"-Wno-unused-function",
				"-Wno-unused-but-set-variable",
				"-Wno-unused-parameter",
				"-Wno-incompatible-pointer-types-discards-qualifiers",
				"-Wno-discarded-qualifiers",
				"-Wno-missing-field-initializers",
				"-Wno-old-style-declaration",
				"-Wno-unknown-warning-option",
		}
		linkoptions { "-pthread" }

	-- whole project
	project "sfDB5"
		kind "ConsoleApp"
		language "C"
		files { "*.c", "*.h" }
		includedirs { "../deps/sort" }
		links { "kv", "pcre" }
		

	-- choose kv-backend
	newoption {
		trigger = "kv",
		value = "kv-backend",
		description = "Choose a key-value storage backend",
		allowed = {
			{ "rocksdb", "RocksDB" },
			{ "sophia", "Sophia" }
		}
	}

	if not _OPTIONS["kv"] then
		_OPTIONS["kv"] = "sophia"
	end

	project "kv"
		kind "StaticLib"
		language "C"
		files { "kv/*.h" }

		configuration "rocksdb"
			links { "rocksdb" }		
			files { "kv/*rocksdb*" }

		configuration "sophia"
			files { "../deps/sophia/sophia.*", "kv/*sophia*" }
