solution "sfDB5"
	configurations { "Debug", "Release" }

	configuration "Debug"
		defines { "_GNU_SOURCE" }
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-pthread" }
		buildoptions { "-Wno-pointer-sign", "-Wno-unused-function",
				"-Wno-unused-but-set-variable" }
		linkoptions { "-pthread" }

	configuration "Release"
		defines { "_GNU_SOURCE" }
		buildoptions { "-pthread", "-march=native", "-O2" }
		buildoptions { "-Wno-pointer-sign", "-Wno-unused-function",
				"-Wno-unused-but-set-variable" }
		linkoptions { "-pthread" }

	-- whole project
	project "sfDB5"
		kind "ConsoleApp"
		language "C"
		files { "*.c", "*.h" }
		includedirs { "../deps/sort" }
		links { "backend" }
		

	-- choose kv-backend
	newoption {
		trigger = "backend",
		value = "storage-backend",
		description = "Choose a key-value storage backend",
		allowed = {
			{ "rocksdb", "RocksDB" }
		}
	}

	if not _OPTIONS["backend"] then
		_OPTIONS["backend"] = "rocksdb"
	end

	project "backend"
		kind "StaticLib"
		language "C"		
		files { "backend/*.h" }

		configuration "rocksdb"
			links { "rocksdb" }			
			files { "backend/*rocksdb*" }
