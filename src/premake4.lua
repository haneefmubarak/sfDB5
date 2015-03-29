solution "sfDB5"
	configurations { "Debug", "Release" }

	configuration "Debug"
		defines { "_GNU_SOURCE" }
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-pthread" }
		buildoptions {	"-Wno-pointer-sign",
				"-Wno-unused-function",
				"-Wno-unused-but-set-variable",
				"-Wno-unknown-warning",
				"-Wno-unknown-warning-option" }
		linkoptions { "-pthread" }

	configuration "Release"
		defines { "_GNU_SOURCE" }
		buildoptions { "-pthread", "-march=native", "-O2" }
		buildoptions {	"-Wno-pointer-sign",
				"-Wno-unused-function",
				"-Wno-unused-but-set-variable",
				"-Wno-unknown-warning",
				"-Wno-unknown-warning-option" }
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
			{ "rocksdb", "RocksDB" }
		}
	}

	if not _OPTIONS["kv"] then
		_OPTIONS["kv"] = "rocksdb"
	end

	project "kv"
		kind "StaticLib"
		language "C"		
		files { "kv/*.h" }

		configuration "rocksdb"
			links { "rocksdb" }			
			files { "kv/*rocksdb*" }
