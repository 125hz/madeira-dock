"""Exercise the public app's source-leak guards in an isolated Git fixture."""
from pathlib import Path
import subprocess
import tempfile

app = Path(__file__).resolve().parents[2] / 'Madeira'
hooks = app / '.githooks'
with tempfile.TemporaryDirectory(prefix='dock-privacy-') as name:
    repo = Path(name)
    def git(*args):
        return subprocess.run(['git', *args], cwd=repo, check=True, capture_output=True, text=True).stdout.strip()
    def check(hook, expected, stdin=None):
        result = subprocess.run(['sh', str(hooks / hook), 'origin', 'https://github.com/125hz/Madeira.git'],
                                cwd=repo, input=stdin, capture_output=True, text=True)
        assert result.returncode == expected, (hook, result.returncode, result.stderr)
    git('init', '-q')
    git('config', 'user.name', 'Fixture')
    git('config', 'user.email', 'fixture@example.invalid')
    (repo / 'dockhost.exe').write_bytes(b'MZ\x00fixture')
    git('add', 'dockhost.exe')
    check('pre-commit', 0)
    git('commit', '-qm', 'Binary fixture')
    (repo / 'renamed.c').write_text('/* MADEIRA_DOCK_PRIVATE_SOURCE */\n')
    git('add', 'renamed.c')
    check('pre-commit', 1)
    git('reset', '-q', 'HEAD', '--', 'renamed.c')
    source = repo / 'build/steam-host/test.c'
    source.parent.mkdir(parents=True)
    source.write_text('/* source fixture */\n')
    git('add', 'build/steam-host/test.c')
    check('pre-commit', 1)
    git('commit', '-qm', 'Private path fixture')
    git('rm', '-q', 'build/steam-host/test.c')
    git('commit', '-qm', 'Remove fixture')
    check('pre-commit', 0)
    sha = git('rev-parse', 'HEAD')
    check('pre-push', 1, f'refs/heads/test {sha} refs/heads/test {"0" * 40}\n')
print('PASS: binary permitted, source path/renamed marker blocked, deleted source blocked in pushed history')
