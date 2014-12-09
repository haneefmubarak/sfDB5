solution "sfDB5"
	configurations { "Debug", "Release" }

	configuration "Debug"
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-pthread" }
		linkoptions { "-pthread" }

	configuration "Release"
		buildoptions { "-pthread", "-march=native", "-O2" }
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
