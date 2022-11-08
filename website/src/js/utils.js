export function bindPrototypeMethods(obj) {
	const bindAll = (obj, prototype) =>
	{
		if(!(prototype && prototype instanceof Object))
		{
			return;
		}

		bindAll(obj, Object.getPrototypeOf(prototype));

		for(const propName of Object.getOwnPropertyNames(prototype))
		{
			if(propName === 'constructor') continue;

			const prop = prototype[propName];
			if(typeof prop !== 'function') continue;

			obj[propName] = prop.bind(obj);
		}
	};

	bindAll(obj, Object.getPrototypeOf(obj));
}

export function clearMobileKeyboard(callback)
{
	var field = document.createElement('input');
	field.setAttribute('type', 'text');
	document.body.appendChild(field);

	setTimeout(function() {
	    field.focus();
	    setTimeout(function() {
	        field.setAttribute('style', 'display:none;');
	        callback && callback();
	    }, 0);
	}, 50);
}

export function normalizeString(str)
{
	return str.normalize("NFD").replace(/[\u0300-\u036f]/g, "");
}

export function downloadToFile(data, filename, type = 'text/plain')
{
    const file = new Blob([data], {type: type});
    if (window.navigator.msSaveOrOpenBlob) // IE10+
    {
        window.navigator.msSaveOrOpenBlob(file, filename);
    }
    else // Others
    {
        const a = document.createElement("a");
        a.download = filename;
		
		const url = URL.createObjectURL(file);
        a.href = url;

        document.body.appendChild(a);
        
        a.click();
        setTimeout(() => 
        {
            document.body.removeChild(a);
            window.URL.revokeObjectURL(url);  
        }, 0); 
    }
}

window.downloadToFile = downloadToFile

export function onUploadFile(input, callback, allowMultipleFiles = true)
{
	if(allowMultipleFiles)
	{
		input.multiple = 'multiple';
		input.name = 'files[]';
	}
	else
	{
		delete input.multiple;
		input.name = 'file';
	}

	input.addEventListener('change', (e) => 
	{
		if(!e.target.files) return;

		for(const file of e.target.files)
		{
			if (!file)
			{
				continue;
			}

			const reader = new FileReader();
			reader.onload = (e) => 
			{
				const contents = e.target.result;
				callback(contents);
			};
			reader.readAsText(file);

			if(!allowMultipleFiles)
			{
				break;
			}
		}
	}, false);
}

export function makeDiv(type, attributes)
{
    if(typeof type == 'object')
    {
        attributes = type;
        type = 'div';
    }

    const el = document.createElement(type);

    for(const attr in attributes)
    {
        let value = attributes[attr];

        if(attr == 'class' || attr == 'classes' || attr == 'classList')
        {
            if(typeof value == "string")
            {
                value = value.split(' ');
            }

            for(let className of value)
            {
                className = className.trim()
                className && el.classList.add(className.trim());
            }
        }
        else if(attr == 'style')
        {
        	if(typeof value == 'string')
        	{
        		el.style = value;
        	}
        	else
        	{
	            for(const prop in value)
	            {
	                el.style[prop] = value[prop];
	            }
	        }
        }
        else if(attr == 'parent')
        {
            value.appendChild(el);
        }
        else if(attr == 'text')
        {
            el.textContent = value;
        }
        else
        {
            el[attr] = value;
        }
    }

    window.componentHandler && window.componentHandler.upgradeElement(el);

    return el;
}