# Development Tools

## Tools

### `commands`

Execute and show the output of the commands given via stdin.

Each line given to the standard input of this program will be processed. Lines
that start with "exec:" will be executed with the output shown in a new tab.
For instance:
```bash
echo 'exec:ls -l' | ./bin/commands
```
will create a new tab and show the output of running `ls -l`.

## License

Work in this repository is made available under the MPL-2.0 license (see
`LICENSE.txt`).
