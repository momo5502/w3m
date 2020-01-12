nngpp = {
	source = path.join(dependencies.basePath, "nngpp"),
}

function nngpp.import()
	nngpp.includes()
end

function nngpp.includes()
	includedirs {
		path.join(nngpp.source, "include"),
	}
end

function nngpp.project()

end

table.insert(dependencies, nngpp)
